#include "si3stage_core.h"

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

si3stage_core_t::si3stage_core_t(
	std::string _name, io::json _config, scheduler_t *_scheduler) 
	: core_t(_name, _config, _scheduler), 
	fetch("fetch", _config, _scheduler), decode("decode", _config, _scheduler),
	exec("exec", _config, _scheduler), retire("retire", _config, _scheduler),
	squashes("squashes"), flushes("flushes"), jumps("jumps"), branches("branches") {	
	squashes.reset();
	flushes.reset();
	jumps.reset();
	branches.reset();

	io::json branch_config;
	std::string branch_type = "tournament";
	if(config["branch_predictor"].is_object()) {
		branch_config = config["branch_predictor"].object_items();
		JSON_CHECK(string, branch_config["type"], branch_type);
	}
	predictor = branch_predictor_type_map.at(branch_type)(branch_config);

	icache_read_port = create_port<("icache_read_port");
	create_port<("icache_retire_port");
	fetch.connect(decode);
	decode.connect(exec);
	exec.connect(retire);
	ports["mem_retire_port"].connect(fetch["mem_retire_port"], 0);
	ports[""]
}

io::json si3stage_core_t::to_json() const {
	return io::json::merge_objects(core_t::to_json(), 
		squashes, flushes, jumps, branches);
}

void si3stage_core_t::reset() {
	core_t::reset(level);
	predictor->reset();
}

void si3stage_core_t::buffer_insn(hstd::shared_ptr<timed_insn_t> insn) {
	insns.push_back(insn);
	next_insn();
}

void si3stage_core_t::next_insn() {
	if(insns.size() > insn_idx && insns.size() - insn_idx >= 3) {
		values->set_ready(false);
		auto insn = insns[insn_idx];
		insn->idx = retired_idx + insn_idx++;
		auto value = new insn_fetch_value_t(this, insn, clock.get());
		values->push_back(value);
		register_squashed("fetch", value);
	} else {
		values->set_ready(true);
	}
}

#if 0
void si3stage_core_t::process(insn_fetch_value_tvalue) {
	TIME_VIOLATION_CHECK
	count["fetch"].running.inc();
	auto pending_value = new pending_value_t(
		this, new insn_decode_value_t(this, value->data), clock.get() + 1);
	pending_value->add_dep<mem_retire_value_t>([pc_val=pc](mem_retire_value_te) {
		return e->data.addr == pc_val;
	});
	pending_value->add_dep([&](){ return !stages["decode"]; });
	pending_value->add_fini([&](){ next_insn(); });
	register_pending(pending_value);
	register_squashed("fetch", pending_value);
	register_squashed("fetch", pending_value->data);
	values->push_back(pending_value);

	auto read_value = new mem_read_value_t(icache, pc, clock.get());
	values->push_back(read_value);
	pc += 4;
}

void si3stage_core_t::process(insn_decode_value_tvalue) {
	TIME_VIOLATION_CHECK
	check_pending(value);
	count["decode"].running.inc();
	// Make a branch prediction
	bool take_branch = predictor->check_branch(value->data->opc) && 
		predictor->predict(value->data->ws.pc) && !value->data->resolved;
	bool take_jump = check_jump(value->data->opc) && !value->data->resolved;
	
	if(predictor->check_branch(value->data->opc) && !value->data->resolved) {
		branches.inc();
	}

	if(take_jump) jumps.inc();

	if(take_jump || take_branch) {
		value->data->resolved = true;
#if SQUASH_LOG
		std::cerr << "================================================" << std::endl;
		std::cerr << "Squashing pipeline status: fetch (0x";
		std::cerr << std::hex << value->data->insn.bits() << ", 0x" << value->data->ws.pc;
		std::cerr << ")" << std::dec << std::endl;
		std::cerr << "================================================" << std::endl;
#endif
		squashes.inc();
		values->push_back(
			new squash_value_t(this, 
				{.idx=value->data->idx, .stages={"fetch"}}, clock.get()));
	}

	value_base_t *exec_value;
	bool has_vcu = vcu != nullptr;
	bool is_vec = false, is_empty = false, is_start = false;
	bool is_split = false, is_vfence = false;
	if(has_vcu) {
		is_vec = vcu->check_vec(value->data->opc);
		is_empty = vcu->check_empty();
		is_start = vcu->check_start();
		is_split = vcu->check_split(value->data->opc);
		is_vfence = vcu->check_fence(value->data->opc);
	}

	if(is_vec) {
		exec_value = new vec_issue_value_t(vcu, value->data);
		// std::cout << "0x" << std::hex << value->data->ws.pc;
		// std::cout << " " << is_start;
		// std::cout << " " << is_split;
		// std::cout << " " << last_split;
		// std::cout << std::endl;
	} else {
		exec_value = new insn_exec_value_t(this, value->data);
	}

	stages["decode"] = true;
	auto pending_value = new pending_value_t(this, exec_value, clock.get() + 1);
	pending_value->add_fini([&](){ stages["decode"] = false; });

	// Stall the core until vcu is done
	// Cases:
	// vfence: flush the vector pipeline
	// current instruction is vector instructon and the last vector instruction
	//.  was a split point (i.e. store, reduction, permutation)
	// current instruction is vector and vcu is already working on instructions(s) / window
	if(has_vcu && !is_empty && 
		(is_vfence || (is_vec && last_split) || (is_vec && is_start))) {
		pending_value->add_dep<vec_retire_value_t>(
			[&](vec_retire_value_te) {
				return true; 
		});
	}

	if(is_vec) last_split = is_split;

	// Start the vcu because a vfence has been encountered and vcu not already started
	if(has_vcu && !is_empty && !is_start && is_vfence) {
		values->push_back(new vec_start_value_t(vcu, false, clock.get()));	
	}

	for(auto it : value->data->ws.input.regs) {
		values->push_back(new reg_read_value_t(this, it, clock.get()));
		pending_value->add_dep<reg_read_value_t>([it](reg_read_value_te){
			return e->data == it;
		});
	}

	pending_value->add_dep([&]() { return !stages["exec"]; });
	register_pending(pending_value);
	register_squashed("decode", pending_value);
	register_squashed("decode", pending_value->data);
	values->push_back(pending_value);
}

void si3stage_core_t::process(insn_exec_value_tvalue) {
	TIME_VIOLATION_CHECK
	check_pending(value);
	// Check for branch misprediction
	if(predictor->check_branch(value->data->opc)) {
		if(!predictor->check_predict(value->data->ws.pc, value->data->ws.next_pc)) {
			value->data->resolved = true;
			flushes.inc();
			// ADD 2x BUBBLE
#if SQUASH_LOG
			std::cerr << "================================================" << std::endl;
			std::cerr << "Squashing pipeline status: decode, fetch (0x";
			std::cerr << std::hex << (uint32_t)value->data->insn.bits() << ", ";
			std::cerr << "predict: " << predictor->predict(value->data->ws.pc) << ", ";
			std::cerr << "check: " << predictor->check_predict(value->data->ws.pc, value->data->ws.next_pc) << ", ";
			std::cerr << "cur: 0x" << value->data->ws.pc << ", actual: 0x" << value->data->ws.next_pc;
			std::cerr << std::dec << ")" << std::endl;
			std::cerr << "================================================" << std::endl;
#endif
			predictor->update(value->data->ws.pc, value->data->ws.next_pc);
			values->push_back(new squash_value_t(this, 
				{.idx=value->data->idx, .stages={"fetch", "decode"}}, clock.get()));
		} else {
			predictor->update(value->data->ws.pc, value->data->ws.next_pc);
		}
	}
	
	// Effectively commit instruction (as in branch resolved)
	insns.pop_front();
	insn_idx--;
	retired_idx++;
	stages["exec"] = true;

	if(vcu != nullptr) vcu->check_and_set_vl(value->data);

	if(value->data->ws.input.locs.size() > 0 || 
		value->data->ws.output.locs.size() > 0) {
		count["mem_req"].running.inc();
	} else if(check_mul(value->data->opc)){
		count["mul"].running.inc();
	} else {
		count["alu"].running.inc();
	}

	auto pending_value = new pending_value_t(this, 
		new insn_retire_value_t(this, value->data), clock.get() + 1);
	pending_value->add_fini([&](){ stages["exec"] = false; });

	std::vector<pending_value_t*> reg_pending_values;
	for(auto it : value->data->ws.output.regs) {
		if(value->data->ws.input.locs.size() == 0) {
			values->push_back(new reg_write_value_t(this, it, clock.get()));
		} else {
			reg_pending_values.push_back(new pending_value_t(
				this, new reg_write_value_t(this, it), clock.get()));
			register_pending(reg_pending_values.back());
			values->push_back(reg_pending_values.back());
		}
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
			for(auto reg_pending : reg_pending_values) {
				reg_pending->add_dep<mem_retire_value_t>([it](mem_retire_value_te) {
					return e->data.addr == it;
				});
			}
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

void si3stage_core_t::process(insn_retire_value_tvalue) {
	TIME_VIOLATION_CHECK
	retired_insns.running.inc();
}

void si3stage_core_t::process(squash_value_tvalue) {
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

void si3stage_core_t::process(vec_ready_value_tvalue) {
	TIME_VIOLATION_CHECK
	check_pending(value);
}

void si3stage_core_t::process(vec_retire_value_tvalue) {
	TIME_VIOLATION_CHECK
	check_pending(value);
	// std::cout << "Retired: " << std::hex << value->data->ws.pc << std::endl;
}

void si3stage_core_t::process(reg_read_value_tvalue) {
	core_t::process(value);
	if(vcu != nullptr) vcu->check_pending(value);	
}
#endif
