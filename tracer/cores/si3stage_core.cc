#include "si3stage_core.h"

#include <algorithm>

#include "log.h"
#include "ram.h"
#include "working_set.h"
#include "core_event.h"
#include "mem_event.h"
#include "signal_event.h"
#include "pending_event.h"

si3stage_core_t::si3stage_core_t(std::string _name, io::json _config, 
	event_list_t *_events) : core_t(_name, _config, _events) {}

void si3stage_core_t::init() {
	std::string icache_id;
	JSON_CHECK(string, config["icache"], icache_id);
	// assert_msg(children.find(icache_id) != children.end(), "icache not found");
	// icache = static_cast<ram_t *>(children[icache_id]);
}

io::json si3stage_core_t::to_json() const {
	return core_t::to_json();
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
	// auto pending_event = new pending_event_t(
	// 	this, new insn_decode_event_t(this, &event->data), clock.get() + 1);
	// events->push_back(
	// 		pending_event->depends(
	// 			new mem_read_event_t(icache, event->data.ws->pc, clock.get(), event)));
	// events->push_back(pending_event);
}

void si3stage_core_t::process(insn_retire_event_t *event) {
	TIME_VIOLATION_CHECK
	retired_insns.inc();
}

// Does not yet include CSRs
void si3stage_core_t::process(insn_decode_event_t *event) {
	TIME_VIOLATION_CHECK
	// next_insn();
	// auto pending_event = new pending_event_t(this, 
	// 	new insn_retire_event_t(this, &event->data), clock.get() + 1);
	// for(auto it : event->data.ws->input.regs)
	// 	events->push_back(
	// 		pending_event->depends(
	// 			new reg_read_event_t(this, it, clock.get())));
	// for(auto it : event->data.ws->output.regs)
	// 	events->push_back(
	// 		pending_event->depends(
	// 			new reg_write_event_t(this, it, clock.get())));
	// for(auto it : event->data.ws->input.locs) {
	// 	for(auto child : children) {
	// 		auto mem = dynamic_cast<ram_t *>(child.second);
	// 		if(mem == nullptr) continue;
	// 		events->push_back(
	// 			pending_event->depends(
	// 				new mem_read_event_t(mem, it, clock.get(), event)));
	// 	}
	// }
	// for(auto it : event->data.ws->output.locs) {
	// 	for(auto child : children) {
	// 		auto mem = dynamic_cast<ram_t *>(child.second);
	// 		if(mem == nullptr) continue;
	// 		events->push_back(
	// 			pending_event->depends(
	// 				new mem_write_event_t(mem, it, clock.get(), event)));
	// 	}
	// }
	// events->push_back(pending_event);
}

void si3stage_core_t::process(pending_event_t *event) {
	TIME_VIOLATION_CHECK
	if(!event->check()) {
		// Recheck during next cycle
		event->cycle = clock.get() + 1;
		event->ready_gc = false;
		events->push_back(event);
		return;
	}
	event->data->cycle = clock.get() + 1;
	event->ready_gc = true;
	events->push_back(event->data);
}

void si3stage_core_t::process(reg_read_event_t *event) {
	TIME_VIOLATION_CHECK
}

void si3stage_core_t::process(reg_write_event_t *event) {
	TIME_VIOLATION_CHECK
}

void si3stage_core_t::process(ready_event_t *event) {
	TIME_VIOLATION_CHECK
}

void si3stage_core_t::process(stall_event_t *event) {
	TIME_VIOLATION_CHECK
}
