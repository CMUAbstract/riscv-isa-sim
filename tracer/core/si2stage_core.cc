#include "si2stage_core.h"

#include <algorithm>

#include "log.h"
#include "ram.h"
#include "vcu.h"
#include "working_set.h"
#include "core_value.h"
#include "mem_value.h"
#include "pending_value.h"
#include "squash_value.h"
#include "vector_value.h"
#include "branch_predictor.h"

#define SQUASH_LOG 0

si2stage_core_t::si2stage_core_t(std::string _name, io::json _config, 
	value_heap_t *_values) : core_t(_name, _config, _values) {
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
		values->set_ready(false);
		auto insn = insns[insn_idx];
		insn->idx = retired_idx + insn_idx++;
		auto value = new insn_fetch_value_t(this, insn, clock.get());
		values->push_back(value);
	} else {
		values->set_ready(true);
	}
}

void si2stage_core_t::process(insn_fetch_value_tvalue) {
	TIME_VIOLATION_CHECK
	stages["fetch"] = true;
	// Memory access to get instruction
	if(icache == nullptr) {
		for(auto child : children.raw<ram_handler_t *>())
			values->push_back(
					new mem_read_value_t(child.second, pc, clock.get()));
	} else {
		values->push_back(new mem_read_value_t(icache, pc, clock.get()));
	}

	// Determine if branch/jump and if next pc is not pc+4
	bool take_branch = predictor->check_branch(value->data->opc) && 
		predictor->predict(value->data->ws.pc) && !value->data->resolved;
	bool take_jump = check_jump(value->data->opc) && !value->data->resolved;
	if(take_jump || take_branch) {
		insn_idx = value->data->idx - retired_idx + 1;
		pc = insns[insn_idx]->ws.pc;	
	} else pc += 4;

	// Determine if vector instruction
	value_base_t *exec_value;
	bool has_vcu = vcu != nullptr;
	bool is_vec = false, is_empty = false, is_split = false;
	if(has_vcu) {
		is_vec = vcu->check_vec(value->data->opc);
		is_empty = vcu->check_empty();
		is_split = vcu->check_split(value->data->opc);
	}
	if(is_vec) {
		exec_value = new vec_issue_value_t(vcu, value->data);
		last_vec = true;
	} else {
		exec_value = new insn_exec_value_t(this, value->data);
	}

	auto pending_value = new pending_value_t(this, exec_value, clock.get() + 1);
	pending_value->add_fini([&, is_vec](){ 
		stages["fetch"] = false;
		last_vec = is_vec;
	});
	if(has_vcu && !is_empty && ((last_vec && !is_vec) || is_split)) {
		pending_value->add_dep<vec_retire_value_t>([](vec_retire_value_te) { 
			return true; 
		});
	}

	for(auto it : value->data->ws.input.regs) {
		values->push_back(new reg_read_value_t(this, it, clock.get()));
		pending_value->add_dep<reg_read_value_t>([it](reg_read_value_te){
			return e->data == it;
		});
	}

	pending_value->add_dep([&]() { return !stages["exec"]; });
	register_pending(pending_value);
	register_squashed("fetch", pending_value);
	register_squashed("fetch", pending_value->data);
	values->push_back(pending_value);
}

void si2stage_core_t::process(insn_exec_value_tvalue) {
	TIME_VIOLATION_CHECK
	check_pending(value);
	// Check for branch misprediction
	if(predictor->check_branch(value->data->opc) && 
		!predictor->check_predict(value->data->ws.pc, value->data->ws.next_pc)) {
		predictor->update(value->data->ws.pc, value->data->ws.next_pc);
		value->data->resolved = true;
		// ADD 2x BUBBLE
#if SQUASH_LOG
		std::cerr << "================================================" << std::endl;
		std::cerr << "Squashing pipeline status: fetch (0x";
		std::cerr << std::hex << value->data->insn.bits() << std::dec << ")" << std::endl;
		std::cerr << "================================================" << std::endl;
#endif
		values->push_back(
			new squash_value_t(this, 
				{.idx=value->data->idx, .stages={"fetch"}}, clock.get()));
	}
	// Effectively commit instruction (as in branch resolved)
	insns.pop_front();
	insn_idx--;
	retired_idx++;
	stages["exec"] = true;

	if(vcu != nullptr) vcu->check_and_set_vl(value->data);

	auto pending_value = new pending_value_t(this, 
		new insn_retire_value_t(this, value->data), clock.get() + 1);
	pending_value->add_fini([&](){ stages["exec"] = false; });
	for(auto it : value->data->ws.input.regs) {
		values->push_back(new reg_write_value_t(this, it, clock.get()));
		pending_value->add_dep<reg_write_value_t>([it](reg_write_value_te){
			return e->data == it;
		});
	}
	for(auto it : value->data->ws.input.locs) {
		for(auto child : children.raw<ram_handler_t *>()) {
			if(child.second == icache) continue;
			values->push_back(
					new mem_read_value_t(child.second, it, clock.get()));
			pending_value->add_dep<mem_ready_value_t>([it](mem_ready_value_te) {
				return e->data.addr == it;
			});
		}
	}
	for(auto it : value->data->ws.output.locs) {
		for(auto child : children.raw<ram_handler_t *>()) {
			if(child.second == icache) continue;
			values->push_back(
					new mem_write_value_t(child.second, it, clock.get()));
			pending_value->add_dep<mem_ready_value_t>([it](mem_ready_value_te) {
				return e->data.addr == it;
			});
		}
	}
	register_pending(pending_value);
	values->push_back(pending_value);
}

void si2stage_core_t::process(insn_retire_value_tvalue) {
	TIME_VIOLATION_CHECK
	retired_insns.running.inc();
}

void si2stage_core_t::process(squash_value_tvalue) {
	TIME_VIOLATION_CHECK
	for(auto stage : value->data.stages) {
		squash(stage);
		clear_squash(stage);
		stages[stage] = false;
	}
	insn_idx = value->data.idx - retired_idx + 1;
	pc = insns[insn_idx]->ws.pc;
	next_insn();
}

void si2stage_core_t::process(vec_ready_value_tvalue) {
	TIME_VIOLATION_CHECK
}

void si2stage_core_t::process(vec_retire_value_tvalue) {
	TIME_VIOLATION_CHECK
	check_pending(value);
}
