#include "cache.h"

#include "log.h"
#include "event.h"
#include "mem_event.h"
#include "repl_policy.h"

cache_t::cache_t(std::string _name, io::json _config, event_list_t *_events)
	: mem_t(_name, _config, _events) {
	JSON_CHECK(int, config["lines"], lines, 64);
	JSON_CHECK(int, config["line_size"], line_size);
	JSON_CHECK(int, config["sets"], sets, 8);
	JSON_CHECK(int, config["read_latency"], read_latency);
	JSON_CHECK(int, config["write_latency"], write_latency);
	JSON_CHECK(int, config["invalid_latency"], invalid_latency);
	init();
}

void cache_t::init() {
	offset_mask = line_size - 1; 
	set_mask = (sets - 1) << line_size;
	tag_mask = ~(offset_mask | set_mask);
}

void cache_t::process(mem_read_event_t *event) {
	TIME_VIOLATION_CHECK
	if(!access(event)) { // Read Miss
		for(auto child : children) {
			auto mem = dynamic_cast<mem_t *>(child.second);
			if(mem == nullptr) continue;
			events->push_back(
				new mem_read_event_t(
					mem, event->data, clock.get() + read_latency, event));
		}
		for(auto parent : parents) {
			events->push_back(
				new mem_stall_event_t(
					parent.second, clock.get() + read_latency, event));
		}
	}
	for(auto parent : parents) { // Insert in higher-level caches
		auto mem = dynamic_cast<mem_t *>(parent.second);
		if(mem == nullptr) continue;
		events->push_back(
			new mem_insert_event_t(
				mem, event->data, clock.get() + read_latency, event));
	}
	for(auto parent : parents) { // Blocking
		events->push_back(
			new mem_ready_event_t(
				parent.second, event->data, clock.get() + read_latency, event));
	}
}

void cache_t::process(mem_write_event_t *event) {
	TIME_VIOLATION_CHECK
	if(!access(event)) { // Write Miss
		for(auto child : children) {
			auto mem = dynamic_cast<mem_t *>(child.second);
			if(mem == nullptr) continue;
			events->push_back(
				new mem_write_event_t(
					mem, event->data, clock.get() + write_latency, event));
		}
		for(auto parent : parents) { // Blocking
			events->push_back(
				new mem_stall_event_t(
					parent.second, clock.get() + write_latency, event));
		}
	}
	for(auto parent : parents) { // Insert in higher-level caches
		auto mem = dynamic_cast<mem_t *>(parent.second);
		if(mem == nullptr) continue;
		events->push_back(
			new mem_insert_event_t(
				mem, event->data, clock.get() + write_latency, event));
	}
	for(auto parent : parents) { // Blocking
		events->push_back(
			new mem_ready_event_t(
				parent.second, event->data, clock.get() + write_latency, event));
	}
}

void cache_t::process(mem_insert_event_t *event) {
	TIME_VIOLATION_CHECK
	uint32_t set = event->data & set_mask;
	uint32_t tag = event->data & tag_mask;
	uint32_t set_size = lines / sets;
	std::vector<repl_cand_t> cands; // Create a set of candidates
	uint32_t id = set * set_size;
	for(id; id < (set + 1) * set_size; id++) cands.push_back(id);
	id = repl_policy->rank(event, &cands); // find which to replace
	data[id] = event->data * tag_mask; // record new element in cache
	for(auto parent : parents) { // Blocking
		events->push_back(
			new mem_ready_event_t(
				parent.second, event->data, clock.get() + invalid_latency, event));
	}
}

bool cache_t::access(mem_event_t *event) {
	uint32_t set = event->data & set_mask;
	uint32_t tag = event->data & tag_mask;
	uint32_t set_size = lines / sets;
	for(uint32_t id = set * set_size; id < (set + 1) * set_size; id++) {
		if((data[id] & tag_mask) == tag) {
			repl_policy->update(id, event);	
			return true;
		}
	}
	return false;
}

void cache_t::process(mem_ready_event_t *event) {
	TIME_VIOLATION_CHECK
	for(auto parent : parents) {
		events->push_back(
			new mem_ready_event_t(parent.second, event->data, clock.get() + 1, event));
	}
}

void cache_t::process(mem_stall_event_t *event) {
	TIME_VIOLATION_CHECK
	for(auto parent : parents) { // Blocking
		events->push_back(
			new mem_stall_event_t(parent.second, event->data, clock.get() + 1, event));
	}
}

