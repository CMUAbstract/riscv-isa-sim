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
	auto child = children.find<ram_t *>(icache_id);
	assert_msg(child != children.end<ram_t *>(), "icache not found");
	icache = child->second;
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
	{
		auto pending_event = new pending_event_t(
			this, new insn_decode_event_t(this, &event->data), clock.get() + 1);
		auto pc = event->data.ws->pc;
		pending_event->add_dependence<ready_event_t *>([pc](ready_event_t *e) {
			return e->data == pc;
		});
		register_pending(pending_event);
		events->push_back(pending_event);
	}
	{
		auto pending_event = new pending_event_t(this, nullptr, clock.get() + 1);
		pending_event->add_dependence<insn_decode_event_t *>([&](insn_decode_event_t *e) {
			next_insn();
			return true;
		});
		register_pending(pending_event);
		events->push_back(pending_event);
	}
	events->push_back(
			new mem_read_event_t(icache, event->data.ws->pc, clock.get(), event));
}

// Does not yet include CSRs
void si3stage_core_t::process(insn_decode_event_t *event) {
	TIME_VIOLATION_CHECK
	if(status["decode"]) { // Pending event promotion
		event->ready_gc = false;
		auto pending_event = new pending_event_t(this, 
			event, clock.get() + 1);
		pending_event->add_dependence([&](){ return status["decode"]; });
		register_pending(pending_event);
		events->push_back(pending_event);
		return;
	}
	check_pending(event);
	status["decode"] = true;
	auto pending_event = new pending_event_t(this, 
		new insn_exec_event_t(this, &event->data), clock.get() + 1);
	pending_event->add_fini([&](){ status["decode"] = false; });
	for(auto it : event->data.ws->input.regs) {
		events->push_back(new reg_read_event_t(this, it, clock.get()));
		pending_event->add_dependence<reg_read_event_t *>([it](reg_read_event_t *e){
			return e->data == it;
		});
	}
	register_pending(pending_event);
	events->push_back(pending_event);
}

void si3stage_core_t::process(insn_exec_event_t *event) {
	TIME_VIOLATION_CHECK
	if(status["exec"]) { // Pending event promotion
		event->ready_gc = false;
		auto pending_event = new pending_event_t(this, 
			event, clock.get() + 1);
		pending_event->add_dependence([&](){ return status["exec"]; });
		register_pending(pending_event);
		events->push_back(pending_event);
		return;
	}
	status["exec"] = true;
	auto pending_event = new pending_event_t(this, 
		new insn_retire_event_t(this, &event->data), clock.get() + 1);
	pending_event->add_fini([&](){ status["exec"] = false; });
	for(auto it : event->data.ws->input.regs) {
		events->push_back(new reg_write_event_t(this, it, clock.get()));
		pending_event->add_dependence<reg_write_event_t *>([it](reg_write_event_t *e){
			return e->data == it;
		});
	}
	for(auto it : event->data.ws->input.locs) {
		for(auto child : children.raw<ram_handler_t *>()) {
			events->push_back(
					new mem_read_event_t(child.second, it, clock.get(), event));
			pending_event->add_dependence<ready_event_t *>([it](ready_event_t *e) {
				return e->data == it;
			});
		}
	}
	for(auto it : event->data.ws->output.locs) {
		for(auto child : children.raw<ram_handler_t *>()) {
			events->push_back(
					new mem_write_event_t(child.second, it, clock.get(), event));
			pending_event->add_dependence<ready_event_t *>([it](ready_event_t *e) {
				return e->data == it;
			});
		}
	}
	register_pending(pending_event);
	events->push_back(pending_event);
}

void si3stage_core_t::process(insn_retire_event_t *event) {
	TIME_VIOLATION_CHECK
	retired_insns.inc();
}

void si3stage_core_t::process(pending_event_t *event) {
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
	remove_pending(event);
	if(event->data != nullptr) {
		event->data->ready_gc = true;
		event->data->cycle = clock.get();
		events->push_back(event->data);
	}
}

void si3stage_core_t::process(reg_read_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
}

void si3stage_core_t::process(reg_write_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
}

void si3stage_core_t::process(ready_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
}

void si3stage_core_t::process(stall_event_t *event) {
	TIME_VIOLATION_CHECK
}
