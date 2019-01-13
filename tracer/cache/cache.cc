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
	JSON_CHECK(int, config["invalid_latency"], invalid_latency, 1);

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
	dirty.resize(lines, false);
}

io::json cache_t::to_json() const {
	return io::json::merge_objects(
		ram_t::to_json(), accesses, inserts, read_misses, 
		write_misses, read_hits, write_hits);
}

void cache_t::process(mem_read_event_t *event) {
	TIME_VIOLATION_CHECK

	auto bank = get_bank(event->data.addr);
	if(promote_pending(event, [&, bank](){
		if(banks[bank].readers < read_ports_per_bank &&
			banks[bank].total() < ports_per_bank) return false; 
		return true;
	}) != nullptr) {
		banks[bank].readerq++;
		if(banks[bank].readerq <= load_buf_size) {
			for(auto parent : parents.raw<ram_signal_handler_t *>()) {
				events->push_back(
					new mem_ready_event_t(
						parent.second, event->data, clock.get() + 1));
			}
		}
		return;
	}

	reads.inc();

	// Increment readers
	banks[bank].readers++;
	if(banks[bank].readerq > 0) banks[bank].readerq--;
	auto pending_event = new pending_event_t(
		this, nullptr, clock.get() + read_latency);
	pending_event->add_fini([&, bank](){ banks[bank].readers--; });
	register_pending(pending_event);
	events->push_back(pending_event);

	if(!access(event)) { // Read Miss
		read_misses.inc();
		for(auto child : children.raw<ram_t *>()) {
			events->push_back(
				new mem_read_event_t(
					child.second, event->data, clock.get() + 1));
			auto pending_event = new pending_event_t (
				this, new mem_insert_event_t(this, event->data), clock.get() + 1);
			pending_event->add_dep<mem_retire_event_t *>(
				[addr=event->data.addr](mem_retire_event_t *e) {
				return e->data.addr == addr;
			});
		}
		for(auto parent : parents.raw<ram_signal_handler_t *>()) {
			events->push_back(
				new mem_ready_event_t(
					parent.second, event->data, clock.get() + 1));
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
				parent.second, event->data, clock.get() + 1));
		events->push_back(
			new mem_retire_event_t(
				parent.second, event->data, clock.get() + read_latency));
	}
}

void cache_t::process(mem_write_event_t *event) {
	TIME_VIOLATION_CHECK
	
	auto bank = get_bank(event->data.addr);
	if(promote_pending(event, [&, bank](){
		if(banks[bank].writers < write_ports_per_bank &&
			banks[bank].total() < ports_per_bank) return false;
		return true;
	}) != nullptr) {
		banks[bank].writerq++;
		if(banks[bank].writerq <= store_buf_size) {
			for(auto parent : parents.raw<ram_signal_handler_t *>()) {
				events->push_back(
					new mem_ready_event_t(
						parent.second, event->data, clock.get() + 1));
			}
		}
		return;
	}

	writes.inc();

	// Increment writers
	banks[bank].writers++;
	if(banks[bank].writerq > 0) banks[bank].writerq--;
	auto pending_event = new pending_event_t(
		this, nullptr, clock.get() + read_latency);
	pending_event->add_fini([&, bank](){ banks[bank].writers--; });
	register_pending(pending_event);
	events->push_back(pending_event);

	if(!access(event)) { // Write Miss
		write_misses.inc();
		for(auto child : children.raw<ram_t *>()) {
			events->push_back(
				new mem_write_event_t(
					child.second, event->data, clock.get() + 1));
			auto pending_event = new pending_event_t (
				this, new mem_insert_event_t(this, event->data), clock.get() + 1);
			pending_event->add_dep<mem_retire_event_t *>(
				[addr=event->data.addr](mem_retire_event_t *e) {
				return e->data.addr == addr;
			});
		}
		for(auto parent : parents.raw<ram_signal_handler_t *>()) {
			events->push_back(
				new mem_ready_event_t(
					parent.second, event->data, clock.get() + 1));
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
			new mem_ready_event_t(
				parent.second, event->data, clock.get() + 1));
		events->push_back(
			new mem_retire_event_t(
				parent.second, event->data, clock.get() + write_latency));
	}
}

void cache_t::process(mem_insert_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
	auto bank = get_bank(event->data.addr);
	if(promote_pending(event, [&, bank](){
		if(banks[bank].writers < write_ports_per_bank &&
			banks[bank].total() < ports_per_bank) return false;
		return true;
	}) != nullptr) {
		banks[bank].writerq++;
		if(banks[bank].writerq <= store_buf_size) {
			for(auto parent : parents.raw<ram_signal_handler_t *>()) {
				events->push_back(
					new mem_ready_event_t(
						parent.second, event->data, clock.get() + 1));
			}
		}
		return;
	}

	inserts.inc();

	// Increment writers
	banks[bank].writers++;
	if(banks[bank].writerq > 0) banks[bank].writerq--;
	if(!event->data.reader && banks[bank].writerq > 0) banks[bank].writerq--;
	if(event->data.reader && banks[bank].readerq > 0) banks[bank].readerq--;
	auto pending_event = new pending_event_t(
		this, nullptr, clock.get() + read_latency);
	pending_event->add_fini([&, bank](){ banks[bank].writers--; });
	register_pending(pending_event);
	events->push_back(pending_event);

	uint32_t set = get_set(event->data.addr);
	uint32_t tag = get_tag(event->data.addr);
	std::vector<repl_cand_t> cands; // Create a set of candidates
	uint32_t id = set * set_size;
	for(id; id < (set + 1) * set_size; id++) cands.push_back(id);
	id = repl_policy->rank(event, &cands); // find which to replace
	data[id] = event->data.addr & tag_mask; // record new element in cache
	repl_policy->replaced(id); // tell repl policy element replaced

	if(dirty[id]) { // Determine if write back needed
		for(auto child : children.raw<ram_t *>()) {
			events->push_back(
				new mem_write_event_t(
					child.second, event->data, clock.get() + write_latency));
		}
	}
	dirty[id] = false;
	// Does this depend on finished write back?
	for(auto parent : parents.raw<ram_signal_handler_t *>()) {
		events->push_back(
			new mem_retire_event_t(
				parent.second, event->data, clock.get() + invalid_latency));
	}
}

bool cache_t::access(mem_event_t *event) {
	accesses.inc();
	if(accesses.get() > ACCESS_LIMIT && ACCESS_LIMIT_ENABLE) exit(1);
	uint32_t set = get_set(event->data.addr);
	uint32_t tag = get_tag(event->data.addr);
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
	uint32_t set = get_set(event->data.addr);
	uint32_t tag = get_tag(event->data.addr);
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