#include "si3stage_core.h"

#include <algorithm>

#include "log.h"
#include "ram.h"
#include "working_set.h"
#include "core_event.h"
#include "mem_event.h"
#include "signal_event.h"

si3stage_core_t::si3stage_core_t(std::string _name, io::json _config, 
	event_list_t *_events) : core_t(_name, _config, _events) {}

void si3stage_core_t::init() {
	std::string icache_id;
	JSON_CHECK(string, config["icache"], icache_id);
	assert_msg(children.find(icache_id) != children.end(), "icache not found");
	icache = static_cast<ram_t *>(children[icache_id]);
}

io::json si3stage_core_t::to_json() const {
	return component_t::to_json();
}

void si3stage_core_t::buffer_insn(timed_insn_t *insn) {
	insns.push_back(insn);
	next_insn();
}

void si3stage_core_t::next_insn() {
	if(insns.size() >= 3) {
		events->set_ready(false);
		auto i = new insn_fetch_event_t(this, insns.front());
		i->cycle = clock.get();
		delete *insns.begin();
		insns.erase(insns.begin());
		events->push_back(i);
	} else {
		events->set_ready(true);
	}
}

void si3stage_core_t::process(insn_fetch_event_t *event) {
	TIME_VIOLATION_CHECK
	events->push_back(
			new mem_read_event_t(icache, event->data.ws->pc, clock.get(), event));
	action_set_t action_set;
	action_set.locs.push_back(event->data.ws->pc);
	auto pending_event = new pending_event_t(this, action_set, clock.get() + 1);
	pending_event->next_event = new insn_decode_event_t(this, &event->data);
	pending_events.insert(pending_event);
	events->push_back(pending_event);
}

void si3stage_core_t::process(insn_retire_event_t *event) {
	TIME_VIOLATION_CHECK
	retired_insns.inc();
}

// Does not yet include CSRs
void si3stage_core_t::process(insn_decode_event_t *event) {
	TIME_VIOLATION_CHECK
	next_insn();
	for(auto it : event->data.ws->input.regs)
		events->push_back(new reg_read_event_t(this, it, clock.get()));
	for(auto it : event->data.ws->output.regs)
		events->push_back(new reg_write_event_t(this, it, clock.get()));
	action_set_t action_set;
	for(auto it : event->data.ws->input.locs) {
		for(auto child : children) {
			auto mem = dynamic_cast<ram_t *>(child.second);
			if(mem == nullptr) continue;
			action_set.locs.push_back(it);
			events->push_back(new mem_read_event_t(mem, it, clock.get(), event));
		}
	}
	for(auto it : event->data.ws->output.locs) {
		for(auto child : children) {
			auto mem = dynamic_cast<ram_t *>(child.second);
			if(mem == nullptr) continue;
			action_set.locs.push_back(it);
			events->push_back(new mem_write_event_t(mem, it, clock.get(), event));
		}
	}
	auto pending_event = new pending_event_t(this, action_set, clock.get() + 1);
	pending_event->next_event = new insn_retire_event_t(this, &event->data);
	pending_events.insert(pending_event);
	events->push_back(pending_event);
}

void si3stage_core_t::process(si3stage_core_t::pending_event_t *event) {
	TIME_VIOLATION_CHECK
	if(!event->data.empty()) {
		// Recheck during next cycle
		event->cycle = clock.get() + 1;
		event->ready_gc = false;
		events->push_back(event);
		return;
	}
	pending_events.erase(event);
	event->next_event->cycle = clock.get() + 1;
	event->ready_gc = true;
	events->push_back(event->next_event);
}

void si3stage_core_t::process(reg_read_event_t *event) {
	TIME_VIOLATION_CHECK
}

void si3stage_core_t::process(reg_write_event_t *event) {
	TIME_VIOLATION_CHECK
}

void si3stage_core_t::process(ready_event_t *event) {
	TIME_VIOLATION_CHECK
	for(auto e : pending_events) {
		auto loc = std::find(e->data.locs.begin(), e->data.locs.end(), event->data);
		if(loc != e->data.locs.end()) {
			e->data.locs.erase(loc);
		}
	}
}

void si3stage_core_t::process(stall_event_t *event) {
	TIME_VIOLATION_CHECK
}
