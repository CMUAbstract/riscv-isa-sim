#include "cache.h"

#include <bitset>
#include <string>

#include "log.h"
#include "event.h"
#include "mem_event.h"
#include "repl_policy.h"

#define ACCESS_LIMIT 20
#define ACCESS_LIMIT_ENABLE 0

cache_t::cache_t(std::string _name, io::json _config, event_heap_t *_events)
	: ram_t(_name, _config, _events), accesses("accesses"), inserts("inserts"),
	read_misses("read_misses"), write_misses("write_misses"),
	read_hits("read_hits"), write_hits("write_hits") {
	JSON_CHECK(int, config["lines"], lines, 64);
	JSON_CHECK(int, config["line_size"], line_size, 4);
	JSON_CHECK(int, config["sets"], sets, 8);
	JSON_CHECK(int, config["banks"], bank_count, 1);
	JSON_CHECK(int, config["read_latency"], read_latency, 1);
	JSON_CHECK(int, config["write_latency"], write_latency, 1);
	JSON_CHECK(int, config["invalid_latency"], invalid_latency, 1);
	JSON_CHECK(int, config["read_ports"], read_ports, 0);
	JSON_CHECK(int, config["write_ports"], write_ports, 0);
	JSON_CHECK(int, config["ports"], ports, 0);
	
	// Calculate number of ports and ports/bank
	assert_msg((ports > 0 || (read_ports > 0 && write_ports > 0)) &&
		!(ports > 0 && (read_ports > 0 && write_ports > 0)), 
			"can't define both total # ports and read and write ports");
	assert_msg(ports % bank_count == 0 
		|| (read_ports % bank_count == 0 && write_ports % bank_count == 0),
		"port - bank mismatch");
	banks.resize(bank_count, std::make_tuple(0, 0));
	if(ports > 0) total_ports = true;
	ports_per_bank = ports / bank_count;
	read_ports_per_bank = read_ports / bank_count;
	write_ports_per_bank = write_ports / bank_count;	

	std::string which_repl_policy;
	JSON_CHECK(string, config["repl_policy"], which_repl_policy);
	if(which_repl_policy.size() == 0) {
		repl_policy = new lru_repl_policy_t(lines); 
	} else {
		repl_policy = repl_policy_type_map.at(which_repl_policy)(lines); 
	}

	offset_mask = line_size - 1; 
	uint32_t idx = line_size;
	while (idx >>= 1) ++set_offset;
	set_size = lines / sets;
	set_mask = (sets - 1) << set_offset;
	bank_mask = (bank_count - 1) << set_offset;
	tag_mask = ~(offset_mask | set_mask);
	data.resize(lines, 0);
	dirty.resize(lines, false);

	// Stats
	accesses.reset();
	inserts.reset();
	read_misses.reset();
	write_misses.reset();
	read_hits.reset();
	write_hits.reset();
#if 0
	std::cout << name << std::endl;
	std::cout << "offset_mask: " << std::bitset<32>(offset_mask) << std::endl;
	std::cout << "set_mask: " << std::bitset<32>(set_mask) << std::endl;
	std::cout << "set_offset: " << set_offset << std::endl;
	std::cout << "tag_mask: " << std::bitset<32>(tag_mask) << std::endl;
	std::cout << "line_size: " << line_size << std::endl;
	std::cout << "lines: " << lines << std::endl;
#endif
}

void cache_t::reset() {
	ram_t::reset();
	data.resize(lines, 0);
	dirty.resize(lines, false);
}

io::json cache_t::to_json() const {
	return io::json::merge_objects(
		ram_t::to_json(), accesses, inserts, read_misses, 
		write_misses, read_hits, write_hits);
}

void cache_t::process(mem_read_event_t *event) {
	TIME_VIOLATION_CHECK

	auto bank = get_bank(event->data);
	if(promote_pending(event, [&, bank](){
		if(total_ports && std::get<0>(banks[bank]) < ports_per_bank) return false;
		else if(std::get<0>(banks[bank]) < read_ports_per_bank) return false; 
		return true;
	}) != nullptr) return;

	reads.inc();

	// Increment readers
	std::get<0>(banks[bank])++;
	auto pending_event = new pending_event_t(
		this, nullptr, clock.get() + read_latency);
	pending_event->add_fini([&](){ std::get<0>(banks[bank])--; });
	register_pending(pending_event);
	events->push_back(pending_event);

	if(!access(event)) { // Read Miss
		read_misses.inc();
		for(auto child : children.raw<ram_t *>()) {
			events->push_back(
				new mem_read_event_t(
					child.second, event->data, clock.get() + read_latency));
		}
		auto loc = event->data;
		pending_event->add_dep<mem_insert_event_t *>([loc](mem_insert_event_t *e){
			return e->data == loc;
		});
		for(auto parent : parents.raw<ram_signal_handler_t *>()) {
			events->push_back(
				new mem_ready_event_t(
					parent.second, event->data, clock.get() + write_latency));
		}
		return;
	}
	read_hits.inc();
	for(auto parent : parents.raw<ram_t *>()) { // Insert in higher-level caches
		events->push_back(
			new mem_insert_event_t(
				parent.second, event->data, clock.get() + read_latency));
	}
	for(auto parent : parents.raw<ram_signal_handler_t *>()) { // Blocking
		events->push_back(
			new mem_ready_event_t(
				parent.second, event->data, clock.get() + read_latency));
	}
}

void cache_t::process(mem_write_event_t *event) {
	TIME_VIOLATION_CHECK
	auto bank = get_bank(event->data);

	if(promote_pending(event, [&, bank](){
		if(total_ports && std::get<0>(banks[bank]) < ports_per_bank) return false;
		else if(std::get<1>(banks[bank]) < write_ports_per_bank) return false; 
		return true;
	}) != nullptr) return;

	writes.inc();

	// Increment writers
	if(total_ports) std::get<0>(banks[bank])++;
	else std::get<1>(banks[bank])++;
	auto pending_event = new pending_event_t(
		this, nullptr, clock.get() + read_latency);
	pending_event->add_fini([&](){ 
		if(total_ports) std::get<0>(banks[bank])--;
		else std::get<1>(banks[bank])--;
	});
	register_pending(pending_event);
	events->push_back(pending_event);

	if(!access(event)) { // Write Miss
		write_misses.inc();
		for(auto child : children.raw<ram_t *>()) {
			events->push_back(
				new mem_write_event_t(
					child.second, event->data, clock.get() + read_latency));
		}
		auto loc = event->data;
		pending_event->add_dep<mem_insert_event_t *>([loc](mem_insert_event_t *e){
			return e->data == loc;
		});
		for(auto parent : parents.raw<ram_signal_handler_t *>()) {
			events->push_back(
				new mem_ready_event_t(
					parent.second, event->data, clock.get() + read_latency));
		}
		return;
	}

	write_hits.inc();
	set_dirty(event); // Mark line as dirty
	for(auto parent : parents.raw<ram_t *>()) { // Insert in higher-level caches
		events->push_back(
			new mem_insert_event_t(
				parent.second, event->data, clock.get() + write_latency));
	}
	for(auto parent : parents.raw<ram_signal_handler_t *>()) {
		events->push_back(
			new mem_retire_event_t(
				parent.second, event->data, clock.get() + write_latency));
	}
}

void cache_t::process(mem_insert_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
	auto bank = get_bank(event->data);

	if(promote_pending(event, [&, bank](){
		if(total_ports && std::get<0>(banks[bank]) < ports_per_bank) return false;
		else if(std::get<1>(banks[bank]) < write_ports_per_bank) return false; 
		return true;
	}) != nullptr) return;

	writes.inc();

	// Increment writers
	if(total_ports) std::get<0>(banks[bank])++;
	else std::get<1>(banks[bank])++;
	auto pending_event = new pending_event_t(
		this, nullptr, clock.get() + read_latency);
	pending_event->add_fini([&](){ 
		if(total_ports) std::get<0>(banks[bank])--;
		else std::get<1>(banks[bank])--;
	});
	register_pending(pending_event);
	events->push_back(pending_event);

	uint32_t set = get_set(event->data);
	uint32_t tag = get_tag(event->data);
	std::vector<repl_cand_t> cands; // Create a set of candidates
	uint32_t id = set * set_size;
	for(id; id < (set + 1) * set_size; id++) cands.push_back(id);
	id = repl_policy->rank(event, &cands); // find which to replace
	data[id] = event->data & tag_mask; // record new element in cache
	repl_policy->replaced(id); // tell repl policy element replaced

	if(dirty[id]) { // Determine if write back needed
		for(auto child : children.raw<ram_t *>()) {
			events->push_back(
				new mem_write_event_t(
					child.second, event->data, clock.get() + write_latency));
		}
	} else {
		for(auto parent : parents.raw<ram_signal_handler_t *>()) { // Blocking
			events->push_back(
				new mem_retire_event_t(
					parent.second, event->data, clock.get() + invalid_latency));
		}
	}
	dirty[id] = false;
	for(auto parent : parents.raw<ram_t *>()) {
		events->push_back(
			new mem_insert_event_t(
				parent.second, event->data, clock.get() + invalid_latency));
	}
}

bool cache_t::access(mem_event_t *event) {
	accesses.inc();
	if(accesses.get() > ACCESS_LIMIT && ACCESS_LIMIT_ENABLE) exit(1);
	uint32_t set = get_set(event->data);
	uint32_t tag = get_tag(event->data);
	for(uint32_t id = set * set_size; id < (set + 1) * set_size; id++) {
		if((data[id] & tag_mask) == tag) {
			repl_policy->update(id, event);
#if 0
			std::cout << "	HIT: ";
			std::cout << name;
			std::cout << " => access (set: 0x" << std::hex << set;
			std::cout << " tag: 0x" << tag << ")" << std::endl;
#endif
			return true;
		}
	}
#if 0
	std::cout << "	MISS: ";
	std::cout << name;
	std::cout << " => access (set: 0x" << std::hex << set;
	std::cout << " tag: 0x" << tag << ")" << std::endl;
#endif
	return false;
}

void cache_t::set_dirty(mem_event_t *event) {
	uint32_t set = get_set(event->data);
	uint32_t tag = get_tag(event->data);
	for(uint32_t id = set * set_size; id < (set + 1) * set_size; id++) 
		dirty[id] = true;
}

uint32_t cache_t::get_set(addr_t addr) {
	return (addr & set_mask) >> set_offset;
}

uint32_t cache_t::get_tag(addr_t addr) {
	return addr & tag_mask;
}

uint32_t cache_t::get_bank(addr_t addr) {
	return (addr & bank_mask) >> set_offset;
}

void cache_t::process(pending_event_t *event) {
	TIME_VIOLATION_CHECK
	if(!event->resolved()) {
		// Recheck during next cycle
		event->cycle = clock.get() + 1;
		event->ready_gc = false;
		events->push_back(event);
		return;
	}
	event->finish();
	event->ready_gc = true;
	if(event->data != nullptr) {
		event->data->ready_gc = true;
		event->data->cycle = clock.get();
		events->push_back(event->data);
		event->data = nullptr;
	}	
}
