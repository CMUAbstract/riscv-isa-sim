#include "si3stage_core.h"

#include <algorithm>

#include "log.h"
#include "ram.h"
#include "working_set.h"
#include "core_event.h"
#include "mem_event.h"
#include "signal_event.h"
#include "pending_event.h"
#include "squash_event.h"
#include "branch_predictor.h"

si3stage_core_t::si3stage_core_t(std::string _name, io::json _config, 
	event_hmap_t *_events) : core_t(_name, _config, _events) {
	io::json branch_config;
	std::string branch_type = "tournament";
	if(config["branch_predictor"].is_object()) {
		branch_config = config["branch_predictor"].object_items();
		JSON_CHECK(string, branch_config["type"], branch_type);
	}
	predictor = branch_predictor_type_map.at(branch_type)(branch_config);
}

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

void si3stage_core_t::buffer_insn(shared_ptr_t<timed_insn_t> insn) {
	insns.push_back(insn);
	next_insn();
}

void si3stage_core_t::next_insn() {
	if(insns.size() > insn_idx && insns.size() - insn_idx >= 3) {
		events->set_ready(false);
		auto insn = insns[insn_idx++];
		auto event = new insn_fetch_event_t(this, insn, clock.get());
		events->push_back(event);
		state["fetch"].issued.insert(event);
	} else {
		events->set_ready(true);
	}
}

void si3stage_core_t::process(insn_fetch_event_t *event) {
	TIME_VIOLATION_CHECK
	auto pending_event = new pending_event_t(
		this, new insn_decode_event_t(this, event->data), clock.get() + 1);
	auto pc = event->data->ws.pc;
	pending_event->add_dependence<ready_event_t *>([pc](ready_event_t *e) {
		return e->data == pc;
	});
	pending_event->add_fini([&](){
		if(!state["decode"].status) next_insn();
	});
	register_pending(pending_event);
	events->push_back(pending_event);
	state["fetch"].issued.insert(pending_event);

	auto read_event = new mem_read_event_t(icache, event->data->ws.pc, clock.get());
	events->push_back(read_event);
}

// Does not yet include CSRs
void si3stage_core_t::process(insn_decode_event_t *event) {
	TIME_VIOLATION_CHECK
	if(state["decode"].status) { // Pending event promotion
		event->ready_gc = false;
		auto pending_event = new pending_event_t(this, 
			event, clock.get() + 1);
		pending_event->add_dependence([&](){ return !state["decode"].status; });
		register_pending(pending_event);
		events->push_back(pending_event);
		state["fetch"].issued.insert(pending_event);
		return;
	}
	check_pending(event);
	// Make a branch prediction
	if(predictor->check_branch(event->data->opc) && 
		predictor->predict(event->data->ws.pc)) {
		events->push_back(
			new squash_event_t(this, {"fetch"}, clock.get()));
		return;
	}
	state["decode"].status = true;
	auto pending_event = new pending_event_t(this, 
		new insn_exec_event_t(this, event->data), clock.get() + 1);
	pending_event->add_fini([&](){ state["decode"].status = false; });
	for(auto it : event->data->ws.input.regs) {
		events->push_back(new reg_read_event_t(this, it, clock.get()));
		pending_event->add_dependence<reg_read_event_t *>([it](reg_read_event_t *e){
			return e->data == it;
		});
	}
	register_pending(pending_event);
	events->push_back(pending_event);
	state["decode"].issued.insert(pending_event);
}

void si3stage_core_t::process(insn_exec_event_t *event) {
	TIME_VIOLATION_CHECK
	if(state["exec"].status) { // Pending event promotion
		event->ready_gc = false;
		auto pending_event = new pending_event_t(this, 
			event, clock.get() + 1);
		pending_event->add_dependence([&](){ return !state["exec"].status; });
		register_pending(pending_event);
		events->push_back(pending_event);
		state["decode"].issued.insert(pending_event);
		return;
	}
	// Check for branch misprediction
	if(predictor->check_branch(event->data->opc) && 
		predictor->check_predict(event->data->ws.pc, event->data->ws.next_pc)) {
		predictor->update(event->data->ws.pc, event->data->ws.next_pc);
		// ADD 2x BUBBLE
		events->push_back(
			new squash_event_t(this, {"fetch", "decode"}, clock.get()));
		return;
	}
	// Effectively commit instruction (as in branch resolved)
	insns.erase(insns.begin());
	insn_idx--;
	state["exec"].status = true;
	auto pending_event = new pending_event_t(this, 
		new insn_retire_event_t(this, event->data), clock.get() + 1);
	pending_event->add_fini([&](){ state["exec"].status = false; });
	for(auto it : event->data->ws.input.regs) {
		events->push_back(new reg_write_event_t(this, it, clock.get()));
		pending_event->add_dependence<reg_write_event_t *>([it](reg_write_event_t *e){
			return e->data == it;
		});
	}
	for(auto it : event->data->ws.input.locs) {
		for(auto child : children.raw<ram_handler_t *>()) {
			if(child.second == icache) continue;
			events->push_back(
					new mem_read_event_t(child.second, it, clock.get()));
			pending_event->add_dependence<ready_event_t *>([it](ready_event_t *e) {
				return e->data == it;
			});
		}
	}
	for(auto it : event->data->ws.output.locs) {
		for(auto child : children.raw<ram_handler_t *>()) {
			if(child.second == icache) continue;
			events->push_back(
					new mem_write_event_t(child.second, it, clock.get()));
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
		event->data = nullptr;
	}
}

void si3stage_core_t::process(squash_event_t *event) {
	TIME_VIOLATION_CHECK
	for(auto stage : event->data) {
#if 0
		std::cout << "Squashing pipeline state: " << stage << std::endl;
		std::cout << "==============================" << std::endl;
#endif
		auto issued = state[stage].issued.begin();
		while(issued != state[stage].issued.end()) {
			auto it = events->begin();
			while(it != events->end()) { // HERE
				/*if(uuid_compare((*it), *issued) == 0) { 
					(*it)->squashed = true;
					auto pending = dynamic_cast<pending_event_t*>(*it);
					if(pending != nullptr) remove_pending(pending);
				}*/
				it++;
			}
			issued = state[stage].issued.erase(issued);
		}
		state[stage].status = false;
	}
	insn_idx -= event->data.size();
	next_insn();
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
