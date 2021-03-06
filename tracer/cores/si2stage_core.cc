#include "si2stage_core.h"

#include <algorithm>

#include "log.h"
#include "ram.h"
#include "vcu.h"
#include "working_set.h"
#include "core_event.h"
#include "mem_event.h"
#include "pending_event.h"
#include "squash_event.h"
#include "vector_event.h"
#include "branch_predictor.h"

#define SQUASH_LOG 0

si2stage_core_t::si2stage_core_t(std::string _name, io::json _config, 
	event_heap_t *_events) : core_t(_name, _config, _events) {
	io::json branch_config;
	std::string branch_type = "tournament";
	if(config["branch_predictor"].is_object()) {
		branch_config = config["branch_predictor"].object_items();
		JSON_CHECK(string, branch_config["type"], branch_type);
	}
	predictor = branch_predictor_type_map.at(branch_type)(branch_config);
}

void si2stage_core_t::init() {
	{
		std::string id;
		JSON_CHECK(string, config["icache"], id);
		auto child = children.find<ram_t *>(id);
		if(child != children.end<ram_t *>()) icache = child->second;
	}
	{
		std::string id;
		JSON_CHECK(string, config["vcu"], id);
		if(id.size() != 0) {
			auto child = children.find<vcu_t *>(id);
			vcu = child->second;
			vcu->set_core(this);
		}
	}
}

io::json si2stage_core_t::to_json() const {
	return core_t::to_json();
}

void si2stage_core_t::reset(reset_level_t level) {
	core_t::reset(level);
	predictor->reset();
	last_vec = false;
}

void si2stage_core_t::buffer_insn(hstd::shared_ptr<timed_insn_t> insn) {
	insns.push_back(insn);
	next_insn();
}

void si2stage_core_t::next_insn() {
	if(insns.size() > insn_idx && insns.size() - insn_idx >= 3) {
		events->set_ready(false);
		auto insn = insns[insn_idx];
		insn->idx = retired_idx + insn_idx++;
		auto event = new insn_fetch_event_t(this, insn, clock.get());
		events->push_back(event);
	} else {
		events->set_ready(true);
	}
}

void si2stage_core_t::process(insn_fetch_event_t *event) {
	TIME_VIOLATION_CHECK
	stages["fetch"] = true;
	// Memory access to get instruction
	if(icache == nullptr) {
		for(auto child : children.raw<ram_handler_t *>())
			events->push_back(
					new mem_read_event_t(child.second, pc, clock.get()));
	} else {
		events->push_back(new mem_read_event_t(icache, pc, clock.get()));
	}

	// Determine if branch/jump and if next pc is not pc+4
	bool take_branch = predictor->check_branch(event->data->opc) && 
		predictor->predict(event->data->ws.pc) && !event->data->resolved;
	bool take_jump = check_jump(event->data->opc) && !event->data->resolved;
	if(take_jump || take_branch) {
		insn_idx = event->data->idx - retired_idx + 1;
		pc = insns[insn_idx]->ws.pc;	
	} else pc += 4;

	// Determine if vector instruction
	event_base_t *exec_event;
	bool has_vcu = vcu != nullptr;
	bool is_vec = false, is_empty = false, is_split = false;
	if(has_vcu) {
		is_vec = vcu->check_vec(event->data->opc);
		is_empty = vcu->check_empty();
		is_split = vcu->check_split(event->data->opc);
	}
	if(is_vec) {
		exec_event = new vec_issue_event_t(vcu, event->data);
		last_vec = true;
	} else {
		exec_event = new insn_exec_event_t(this, event->data);
	}

	auto pending_event = new pending_event_t(this, exec_event, clock.get() + 1);
	pending_event->add_fini([&, is_vec](){ 
		stages["fetch"] = false;
		last_vec = is_vec;
	});
	if(has_vcu && !is_empty && ((last_vec && !is_vec) || is_split)) {
		pending_event->add_dep<vec_retire_event_t *>([](vec_retire_event_t *e) { 
			return true; 
		});
	}

	for(auto it : event->data->ws.input.regs) {
		events->push_back(new reg_read_event_t(this, it, clock.get()));
		pending_event->add_dep<reg_read_event_t *>([it](reg_read_event_t *e){
			return e->data == it;
		});
	}

	pending_event->add_dep([&]() { return !stages["exec"]; });
	register_pending(pending_event);
	register_squashed("fetch", pending_event);
	register_squashed("fetch", pending_event->data);
	events->push_back(pending_event);
}

void si2stage_core_t::process(insn_exec_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
	// Check for branch misprediction
	if(predictor->check_branch(event->data->opc) && 
		!predictor->check_predict(event->data->ws.pc, event->data->ws.next_pc)) {
		predictor->update(event->data->ws.pc, event->data->ws.next_pc);
		event->data->resolved = true;
		// ADD 2x BUBBLE
#if SQUASH_LOG
		std::cerr << "================================================" << std::endl;
		std::cerr << "Squashing pipeline status: fetch (0x";
		std::cerr << std::hex << event->data->insn.bits() << std::dec << ")" << std::endl;
		std::cerr << "================================================" << std::endl;
#endif
		events->push_back(
			new squash_event_t(this, 
				{.idx=event->data->idx, .stages={"fetch"}}, clock.get()));
	}
	// Effectively commit instruction (as in branch resolved)
	insns.pop_front();
	insn_idx--;
	retired_idx++;
	stages["exec"] = true;

	if(vcu != nullptr) vcu->check_and_set_vl(event->data);

	auto pending_event = new pending_event_t(this, 
		new insn_retire_event_t(this, event->data), clock.get() + 1);
	pending_event->add_fini([&](){ stages["exec"] = false; });
	for(auto it : event->data->ws.input.regs) {
		events->push_back(new reg_write_event_t(this, it, clock.get()));
		pending_event->add_dep<reg_write_event_t *>([it](reg_write_event_t *e){
			return e->data == it;
		});
	}
	for(auto it : event->data->ws.input.locs) {
		for(auto child : children.raw<ram_handler_t *>()) {
			if(child.second == icache) continue;
			events->push_back(
					new mem_read_event_t(child.second, it, clock.get()));
			pending_event->add_dep<mem_ready_event_t *>([it](mem_ready_event_t *e) {
				return e->data.addr == it;
			});
		}
	}
	for(auto it : event->data->ws.output.locs) {
		for(auto child : children.raw<ram_handler_t *>()) {
			if(child.second == icache) continue;
			events->push_back(
					new mem_write_event_t(child.second, it, clock.get()));
			pending_event->add_dep<mem_ready_event_t *>([it](mem_ready_event_t *e) {
				return e->data.addr == it;
			});
		}
	}
	register_pending(pending_event);
	events->push_back(pending_event);
}

void si2stage_core_t::process(insn_retire_event_t *event) {
	TIME_VIOLATION_CHECK
	retired_insns.running.inc();
}

void si2stage_core_t::process(squash_event_t *event) {
	TIME_VIOLATION_CHECK
	for(auto stage : event->data.stages) {
		squash(stage);
		clear_squash(stage);
		stages[stage] = false;
	}
	insn_idx = event->data.idx - retired_idx + 1;
	pc = insns[insn_idx]->ws.pc;
	next_insn();
}

void si2stage_core_t::process(vec_ready_event_t *event) {
	TIME_VIOLATION_CHECK
}

void si2stage_core_t::process(vec_retire_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
}
