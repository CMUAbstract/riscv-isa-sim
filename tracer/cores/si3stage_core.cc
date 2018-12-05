#include "si3stage_core.h"

#include <algorithm>

#include "log.h"
#include "ram.h"
#include "vcu.h"
#include "working_set.h"
#include "core_event.h"
#include "mem_event.h"
#include "signal_event.h"
#include "pending_event.h"
#include "squash_event.h"
#include "vector_event.h"
#include "branch_predictor.h"

// #define SQUASH_LOG 1

si3stage_core_t::si3stage_core_t(std::string _name, io::json _config, 
	event_heap_t *_events) : core_t(_name, _config, _events) {
	io::json branch_config;
	std::string branch_type = "tournament";
	if(config["branch_predictor"].is_object()) {
		branch_config = config["branch_predictor"].object_items();
		JSON_CHECK(string, branch_config["type"], branch_type);
	}
	predictor = branch_predictor_type_map.at(branch_type)(branch_config);
}

void si3stage_core_t::init() {
	{
		std::string id;
		JSON_CHECK(string, config["icache"], id);
		auto child = children.find<ram_t *>(id);
		assert_msg(child != children.end<ram_t *>(), "icache not found");
		icache = child->second;
	}
	{
		std::string id;
		JSON_CHECK(string, config["vcu"], id);
		if(id.size() != 0) {
			auto child = children.find<vcu_t *>(id);
			vcu = child->second;
		}
	}
}

io::json si3stage_core_t::to_json() const {
	return core_t::to_json();
}

void si3stage_core_t::buffer_insn(hstd::shared_ptr<timed_insn_t> insn) {
	insns.push_back(insn);
	next_insn();
}

void si3stage_core_t::next_insn() {
	if(insns.size() > insn_idx && insns.size() - insn_idx >= 3) {
		events->set_ready(false);
		auto insn = insns[insn_idx];
		insn->idx = retired_idx + insn_idx++;
		auto event = new insn_fetch_event_t(this, insn, clock.get());
		events->push_back(event);
		register_squashed("fetch", event);
	} else {
		events->set_ready(true);
	}
}

void si3stage_core_t::process(insn_fetch_event_t *event) {
	TIME_VIOLATION_CHECK
	auto pending_event = new pending_event_t(
		this, new insn_decode_event_t(this, event->data), clock.get() + 1);
	pending_event->add_dependence<ready_event_t *>([pc_val=pc](ready_event_t *e) {
		return e->data == pc_val;
	});
	pending_event->add_fini([&](){
		if(!state["decode"]) next_insn();
	});
	register_pending(pending_event);
	events->push_back(pending_event);
	register_squashed("fetch", pending_event);

	auto read_event = new mem_read_event_t(icache, pc, clock.get());
	events->push_back(read_event);
	pc += 4;
}

void si3stage_core_t::process(insn_decode_event_t *event) {
	TIME_VIOLATION_CHECK
	if(state["decode"]) { // Pending event promotion
		event->ready_gc = false;
		auto pending_event = new pending_event_t(this, 
			event, clock.get() + 1);
		pending_event->add_dependence([&](){ return !state["decode"]; });
		register_pending(pending_event);
		register_squashed("fetch", pending_event);
		events->push_back(pending_event);
		return;
	}
	check_pending(event);
	// Make a branch prediction
	bool take_branch = predictor->check_branch(event->data->opc) && 
		predictor->predict(event->data->ws.pc) && !event->data->resolved;
	bool take_jump = check_jump(event->data->opc) && !event->data->resolved;
	
	if(take_jump || take_branch) {
		event->data->resolved = true;
#if SQUASH_LOG
		std::cout << "================================================" << std::endl;
		std::cout << "Squashing pipeline state: fetch (0x";
		std::cout << std::hex << event->data->insn.bits() << ", 0x" << event->data->ws.pc;
		std::cout << ")" << std::dec << std::endl;
		std::cout << "================================================" << std::endl;
#endif
		events->push_back(
			new squash_event_t(this, 
				{.idx=event->data->idx, .stages={"fetch"}}, clock.get()));
		return;
	}
	state["decode"] = true;
	auto pending_event = new pending_event_t(this, 
		new insn_exec_event_t(this, event->data), clock.get() + 1);
	pending_event->add_fini([&](){ state["decode"] = false; });
	for(auto it : event->data->ws.input.regs) {
		events->push_back(new reg_read_event_t(this, it, clock.get()));
		pending_event->add_dependence<reg_read_event_t *>([it](reg_read_event_t *e){
			return e->data == it;
		});
	}
	register_pending(pending_event);
	events->push_back(pending_event);
	register_squashed("decode", pending_event);
}

void si3stage_core_t::process(insn_exec_event_t *event) {
	TIME_VIOLATION_CHECK
	if(state["exec"]) { // Pending event promotion
		event->ready_gc = false;
		auto pending_event = new pending_event_t(this, 
			event, clock.get() + 1);
		pending_event->add_dependence([&](){ return !state["exec"]; });
		register_pending(pending_event);
		events->push_back(pending_event);
		register_squashed("decode", pending_event);
		return;
	}
	// Check for branch misprediction
	if(predictor->check_branch(event->data->opc) && 
		!predictor->check_predict(event->data->ws.pc, event->data->ws.next_pc)) {
		predictor->update(event->data->ws.pc, event->data->ws.next_pc);
		// ADD 2x BUBBLE
#if SQUASH_LOG
		std::cout << "================================================" << std::endl;
		std::cout << "Squashing pipeline state: decode, fetch (0x";
		std::cout << std::hex << event->data->insn.bits() << std::dec << ")" << std::endl;
		std::cout << "================================================" << std::endl;
#endif
		events->push_back(
			new squash_event_t(this, 
				{.idx=event->data->idx, .stages={"fetch", "decode"}}, clock.get()));
		return;
	}
	// Effectively commit instruction (as in branch resolved)
	insns.pop_front();
	insn_idx--;
	retired_idx++;
	state["exec"] = true;

	if(vcu != nullptr) {
		vcu->check_and_set_vl(event->data);
		if(vcu->check_vec(event->data->opc)) {
			events->push_back(
				new vector_exec_event_t(vcu, event->data, clock.get()));
			auto pending_event = new pending_event_t(this, 
				new insn_retire_event_t(this, event->data), clock.get() + 1);
			pending_event->add_dependence<vector_ready_event_t *>(
				[pc=event->data->ws.pc](vector_ready_event_t *e){
				return pc == e->data->ws.pc;
			});
			pending_event->add_fini([&](){ state["exec"] = false; });
			register_pending(pending_event);
			events->push_back(pending_event);
			return; // Leave for the VCU to finish
		}
	}

	auto pending_event = new pending_event_t(this, 
		new insn_retire_event_t(this, event->data), clock.get() + 1);
	pending_event->add_fini([&](){ state["exec"] = false; });
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
	if(event->data != nullptr) {
		event->data->ready_gc = true;
		event->data->cycle = clock.get();
		events->push_back(event->data);
		event->data = nullptr;
	}
}

void si3stage_core_t::process(squash_event_t *event) {
	TIME_VIOLATION_CHECK
	for(auto stage : event->data.stages) {
		squash(stage);
		clear_squash(stage);
		state[stage] = false;
	}
	insn_idx = event->data.idx - retired_idx;
	pc = insns[insn_idx]->ws.pc;
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

void si3stage_core_t::process(vector_ready_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
}

void si3stage_core_t::process(vector_retire_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
}
