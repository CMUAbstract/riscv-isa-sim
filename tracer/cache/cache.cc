#include "cache.h"

#include <bitset>
#include <string>

#include "log.h"
#include "event.h"
#include "mem_event.h"
#include "signal_event.h"
#include "repl_policy.h"

#define ACCESS_LIMIT 20
#define ACCESS_LIMIT_ENABLE 0

cache_t::cache_t(std::string _name, io::json _config, event_heap_t *_events)
	: ram_t(_name, _config, _events), accesses("accesses"), inserts("inserts"),
	read_misses("read_misses"), write_misses("write_misses"),
	read_hits("read_hits"), write_hits("write_hits") {
	JSON_CHECK(int, config["lines"], lines, 64);
	JSON_CHECK(int, config["line_size"], line_size);
	JSON_CHECK(int, config["sets"], sets, 8);
	JSON_CHECK(int, config["read_latency"], read_latency);
	JSON_CHECK(int, config["write_latency"], write_latency);
	JSON_CHECK(int, config["invalid_latency"], invalid_latency);
	JSON_CHECK(int, config["ports"], ports);
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
	tag_mask = ~(offset_mask | set_mask);
	data.resize(lines);
	status["read"] = 0;
	status["write"] = 0;
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
	status["read"] = 0;
	status["write"] = 0;
}

io::json cache_t::to_json() const {
	return io::json::merge_objects(
		ram_t::to_json(), accesses, inserts, read_misses, 
		write_misses, read_hits, write_hits);
}

void cache_t::process(mem_read_event_t *event) {
	TIME_VIOLATION_CHECK
	if(status["write"] + status["read"] >= ports) { // Pending event promotion
		event->ready_gc = false;
		auto pending_event = new pending_event_t(this, 
			event, clock.get() + 1);
		pending_event->add_dependence([&](){ 
			return status["write"] + status["read"] < ports; 
		});
		register_pending(pending_event);
		events->push_back(pending_event);
		return;
	}
	reads.inc();
	// Increment read and also queue event to decrement read
	status["read"]++;
	auto pending_event = new pending_event_t(
		this, nullptr, clock.get() + read_latency);
	pending_event->add_fini([&](){
		status["read"]--; 
	});
	register_pending(pending_event);
	events->push_back(pending_event);
	if(!access(event)) { // Read Miss
		read_misses.inc();
#if 0
	std::cout << "	read_miss" << std::endl;
#endif
		for(auto child : children.raw<ram_t *>()) {
			events->push_back(
				new mem_read_event_t(
					child.second, event->data, clock.get() + read_latency));
		}
		return;
	}
	read_hits.inc();
	for(auto parent : parents.raw<ram_t *>()) { // Insert in higher-level caches
		events->push_back(
			new mem_insert_event_t(
				parent.second, event->data, clock.get() + read_latency));
	}
	for(auto parent : parents.raw<signal_handler_t *>()) { // Blocking
		events->push_back(
			new ready_event_t(
				parent.second, event->data, clock.get() + read_latency));
	}
}

void cache_t::process(mem_write_event_t *event) {
	TIME_VIOLATION_CHECK
	if(status["write"] + status["read"] >= ports) { // Pending event promotion
		event->ready_gc = false;
		auto pending_event = new pending_event_t(this, 
			event, clock.get() + 1);
		pending_event->add_dependence([&](){ 
			return status["write"] + status["read"] < ports; 
		});
		register_pending(pending_event);
		events->push_back(pending_event);
		return;
	}
	writes.inc();
	// Increment write and also queue event to decrement write
	status["write"]++;
	auto pending_event = new pending_event_t(
		this, nullptr, clock.get() + write_latency);
	pending_event->add_fini([&](){ status["write"]--; });
	register_pending(pending_event);
	events->push_back(pending_event);
	if(!access(event)) { // Write Miss
		write_misses.inc();
#if 0
	std::cout << "	write_miss" << std::endl;
#endif
		for(auto child : children.raw<ram_t *>()) {
			events->push_back(
				new mem_write_event_t(
					child.second, event->data, clock.get() + write_latency));
		}
		return;
	}
	write_hits.inc();
	for(auto parent : parents.raw<ram_t *>()) { // Insert in higher-level caches
		events->push_back(
			new mem_insert_event_t(
				parent.second, event->data, clock.get() + write_latency));
	}
	for(auto parent : parents.raw<signal_handler_t *>()) { // Blocking
		events->push_back(
			new ready_event_t(
				parent.second, event->data, clock.get() + write_latency));
	}
}

void cache_t::process(mem_insert_event_t *event) {
	TIME_VIOLATION_CHECK
	if(status["write"] + status["read"] >= ports) { // Pending event promotion
		event->ready_gc = false;
		auto pending_event = new pending_event_t(this, 
			event, clock.get() + 1);
		pending_event->add_dependence([&](){ 
			return status["write"] + status["read"] < ports; 
		});
		register_pending(pending_event);
		events->push_back(pending_event);
		return;
	}
	inserts.inc();
	// Increment write and also queue event to decrement write
	status["write"]++;
	auto pending_event = new pending_event_t(
		this, nullptr, clock.get() + write_latency);
	pending_event->add_fini([&](){ status["write"]--; });
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
	for(auto parent : parents.raw<signal_handler_t *>()) { // Blocking
		events->push_back(
			new ready_event_t(
				parent.second, event->data, clock.get() + invalid_latency));
	}
	for(auto parent : parents.raw<ram_t *>()) { // Blocking
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

uint32_t cache_t::get_set(addr_t addr) {
	return (addr & set_mask) >> set_offset;
}

uint32_t cache_t::get_tag(addr_t addr) {
	return addr & tag_mask;
}

void cache_t::process(pending_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending();
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
