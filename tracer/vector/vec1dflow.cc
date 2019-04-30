#include "vec1dflow.h"

#include "mem_value.h"
#include "vector_value.h"
#include "pending_value.h"

vec1dflow_t::vec1dflow_t(std::string _name, io::json _config, 
	value_heap_t *_values) : vcu_t(_name, _config, _values) {
	JSON_CHECK(int, config["window_size"], window_size);
	assert_msg(window_size > 0, "Window size must be greater than zero");
	JSON_CHECK(bool, config["src_forwarding"], src_forwarding);
	JSON_CHECK(bool, config["m2m"], m2m);
	JSON_CHECK(bool, config["forwarding"], forwarding, true)

	track_power("fq");
	track_power("issue");
	track_energy("issue");
	track_energy("forward");

	reg_info.resize(window_size);
	progress_map.resize(window_size, 0);
}

io::json vec1dflow_t::to_json() const {
	return vcu_t::to_json();
}

void vec1dflow_t::reset(reset_level_t level) {
	vcu_t::reset(level);
	// start = false;

	// idx = 0;
	// window_start = 0;
	// active_insn_offset = 0;
	// active_window_size = 0;

	// std::fill(progress_map.begin(), progress_map.end(), 0);
}

void vec1dflow_t::process(vec_issue_value_tvalue) {
	TIME_VIOLATION_CHECK;
	if(promote_pending(value, [&](){
		return !(active_window_size < window_size);
	}) != nullptr) return;

	value->data->idx = idx;

	reg_info[idx].op1f = false;
	reg_info[idx].op2f = false;
	reg_info[idx].resultf = REG;
	reg_info[idx].op1 = -1;
	reg_info[idx].op2 = -1;
	reg_info[idx].result = -1;
	if(value->data->ws.output.vregs.size() > 0) {
		reg_info[idx].result = strip_killed(value->data->insn.rd());
	}

	if(value->data->ws.input.vregs.size() >= 1) {
		reg_info[idx].op1 = strip_killed(value->data->insn.rs1());
		for(uint16_t i = 1; i <= active_window_size; i++) {
			uint16_t window_idx = (window_size + window_start + active_window_size - i);
			window_idx %= window_size;
			if(reg_info[window_idx].result == reg_info[idx].op1) {
				reg_info[window_idx].resultf = BOTH;
				reg_info[idx].op1f = true;
				if(check_killed(value->data->insn.rs1())) {
					reg_info[window_idx].resultf = FORWARD;
				}
				break;
			} else if(src_forwarding) {
				assert_msg(1 == 0, "Source forwarding not yet implemented");
			}
		}
	}

	if(value->data->ws.input.vregs.size() >= 2) {
		reg_info[idx].op2 = strip_killed(value->data->insn.rs2());
		for(uint16_t i = 1; i <= active_window_size; i++) {
			uint16_t window_idx = (window_size + window_start + active_window_size - i);
			window_idx %= window_size;
			if(reg_info[window_idx].result == reg_info[idx].op2) {
				reg_info[window_idx].resultf = BOTH;
				reg_info[idx].op2f = true;
				if(check_killed(value->data->insn.rs2())) {
					reg_info[window_idx].resultf = FORWARD;
				}
				break;
			} else if(src_forwarding) {
				assert_msg(1 == 0, "Source forwarding not yet implemented");
			}
		}
	}
	
	progress_map[idx] = 0;
	count["issue"].running.inc();

	idx = (idx + 1) % window_size;
	empty = false;
	active_window_size++;
	// std::cout << "Issuing: " << std::hex << value->data->ws.pc;
	// std::cout << " " << active_window_size << std::endl;
	if(active_window_size == window_size || check_split(value->data->opc)) {
		values->push_back(new vec_start_value_t(this, false, clock.get()));
	} else { // Issue ready signals
		for(auto parent : parents.raw<vec_signal_handler_t *>()) {
			values->push_back(
				new vec_ready_value_t(parent.second, value->data, clock.get()));
		}
	}

	auto pending_value = new pending_value_t(this,
		new pe_exec_value_t(this, value->data), clock.get());
	pending_value->add_dep<vec_start_value_t>([](vec_start_value_tvalue) {
		return true;
	});

	register_pending(pending_value);
	values->push_back(pending_value);
}

void vec1dflow_t::process(vec_start_value_tvalue) {
	vcu_t::process(value);
	start = true;
	if(active_insn_offset == active_window_size) active_insn_offset = 0;
}

void vec1dflow_t::process(pe_exec_value_tvalue) {
	TIME_VIOLATION_CHECK;
	uint16_t insn_idx = value->data->idx;
	if(promote_pending(value, [&, insn_idx]() {
		uint16_t cur_idx = (window_start + active_insn_offset) % window_size;
		return !(cur_idx == insn_idx && outstanding == 0);
	}) != nullptr) return;

	uint16_t work = vl - progress_map[insn_idx];
	uint16_t cur_progress = progress_map[insn_idx];
	pending_value_tpending_value;
	if(work > lanes) {
		work = lanes;
		pending_value = new pending_value_t(this, 
			new pe_exec_value_t(this, value->data), clock.get() + 1);
		pending_value->add_fini([&, insn_idx, work](){ 
			progress_map[insn_idx] += work;
		});
		if(!start) {
			pending_value->add_dep<vec_start_value_t>(
				[](vec_start_value_te) { return true; });
		}
	} else {
		pending_value = new pending_value_t(this, 
			new pe_ready_value_t(this, value->data), clock.get() + 1);
		pending_value->add_fini([&, insn_idx](){ 
			progress_map[insn_idx] = 0; 
		});
	}

	auto retire_value = new pending_value_t(this, nullptr, clock.get() + 1);
	retire_value->add_fini([&](){ outstanding = 0; });
	outstanding = 1;

	if(check_mul(value->data->opc)) {
		count["mul"].running.inc();
	} else {
		count["alu"].running.inc();
	}

	// Input locations
	if(cur_progress < value->data->ws.input.locs.size()) {
		for(auto child : children.raw<ram_t *>()) {
			
			auto it = std::next(
				value->data->ws.input.locs.begin(), cur_progress);
			auto end = std::next(
				value->data->ws.input.locs.begin(), cur_progress + work);
			std::set<addr_t> locs;
			addr_t line_mask = ~(child.second->get_line_size() - 1);
			while(it != end) {
				locs.insert(*it & line_mask);
				++it;
			}

			for(auto loc : locs) {
				values->push_back(
					new mem_read_value_t(child.second, loc, clock.get()));
				pending_value->add_dep<mem_retire_value_t>(
					[loc](mem_retire_value_te){
					return e->data.addr == loc;
				});
				retire_value->add_dep<mem_retire_value_t>(
					[loc](mem_retire_value_te){
					return e->data.addr == loc;
				});
			}
		}
	}

	// Output locations
	if(cur_progress < value->data->ws.output.locs.size()) {
		for(auto child : children.raw<ram_t *>()) {
			
			auto it = std::next(
				value->data->ws.output.locs.begin(), cur_progress);
			auto end = std::next(
				value->data->ws.output.locs.begin(), cur_progress + work);
			std::set<addr_t> locs;
			addr_t line_mask = ~(child.second->get_line_size() - 1);
			while(it != end) {
				locs.insert(*it & line_mask);
				++it;
			}

			for(auto loc : locs) {
				values->push_back(
					new mem_write_value_t(child.second, loc, clock.get()));
				pending_value->add_dep<mem_retire_value_t>(
					[loc](mem_retire_value_te){
					return e->data.addr == loc;
				});
				retire_value->add_dep<mem_retire_value_t>(
					[loc](mem_retire_value_te){
					return e->data.addr == loc;
				});
			}
		}
	}

	// // Input registers
	if(value->data->ws.input.vregs.size() >= 1) {
		uint8_t reg = reg_info[insn_idx].op1;
		if(!m2m) {
			if(!reg_info[insn_idx].op1f || !forwarding) {
				for(uint16_t i = cur_progress; i < cur_progress + work; i++) {
					values->push_back(new vec_reg_read_value_t(
						this, {.reg=reg, .idx=i}, clock.get()));
					pending_value->add_dep<vec_reg_read_value_t>(
						[reg, i](vec_reg_read_value_te){
							return e->data.reg == reg && e->data.idx == i;
					});
				}
			}
		} else {
			if(!reg_info[insn_idx].op1f || !forwarding) {
				for(auto child : children.raw<ram_t *>()) {
					std::set<addr_t> locs;
					addr_t line_mask = ~(child.second->get_line_size() - 1);
					for(uint16_t i = cur_progress; i < cur_progress + work; i++) {
						locs.insert(((reg << 4) | i) & line_mask);
					}
					for(auto loc : locs) {
						values->push_back(
							new mem_read_value_t(child.second, loc, clock.get()));
						pending_value->add_dep<mem_retire_value_t>(
							[loc](mem_retire_value_te){
							return e->data.addr == loc;
						});
						retire_value->add_dep<mem_retire_value_t>(
							[loc](mem_retire_value_te){
							return e->data.addr == loc;
						});
					}
				}
			} 
		}
	}

	if(value->data->ws.input.vregs.size() >= 2) {
		uint8_t reg = reg_info[insn_idx].op2;
		if(!m2m) {
			if(!reg_info[insn_idx].op2f || !forwarding) {
				for(uint16_t i = cur_progress; i < cur_progress + work; i++) {
					values->push_back(new vec_reg_read_value_t(
						this, {.reg=reg, .idx=i}, clock.get()));
					pending_value->add_dep<vec_reg_read_value_t>(
						[reg, i](vec_reg_read_value_te){
							return e->data.reg == reg && e->data.idx == i;
					});
				}
			}
		} else {
			if(!reg_info[insn_idx].op2f || !forwarding) {
				for(auto child : children.raw<ram_t *>()) {
					std::set<addr_t> locs;
					addr_t line_mask = ~(child.second->get_line_size() - 1);
					for(uint16_t i = cur_progress; i < cur_progress + work; i++) {
						locs.insert(((reg << 4) | i) & line_mask);
					}
					for(auto loc : locs) {
						values->push_back(
							new mem_read_value_t(child.second, loc, clock.get()));
						pending_value->add_dep<mem_retire_value_t>(
							[loc](mem_retire_value_te){
							return e->data.addr == loc;
						});
						retire_value->add_dep<mem_retire_value_t>(
							[loc](mem_retire_value_te){
							return e->data.addr == loc;
						});
					}
				}
			}
		}
	}

	// Output registers
	if(value->data->ws.output.vregs.size() > 0) {
		uint8_t reg = reg_info[insn_idx].result;
		if(!m2m) {
			if(reg_info[insn_idx].resultf == REG ||
				reg_info[insn_idx].resultf == BOTH || !forwarding) {
				for(uint16_t i = cur_progress; i < cur_progress + work; i++) {
					values->push_back(new vec_reg_write_value_t(
						this, {.reg=reg, .idx=i}, clock.get()));
					pending_value->add_dep<vec_reg_write_value_t>(
						[reg, i](vec_reg_write_value_te){
							return e->data.reg == reg && e->data.idx == i;
					});
				}
			}
		} else {
			if(reg_info[insn_idx].resultf == REG ||
				reg_info[insn_idx].resultf == BOTH || !forwarding) {
				for(auto child : children.raw<ram_t *>()) {
					std::set<addr_t> locs;
					addr_t line_mask = ~(child.second->get_line_size() - 1);
					for(uint16_t i = cur_progress; i < cur_progress + work; i++) {
						locs.insert(((reg << 4) | i) & line_mask);
					}
					for(auto loc : locs) {
						values->push_back(
							new mem_write_value_t(child.second, loc, clock.get()));
						pending_value->add_dep<mem_retire_value_t>(
							[loc](mem_retire_value_te){
							return e->data.addr == loc;
						});
						retire_value->add_dep<mem_retire_value_t>(
							[loc](mem_retire_value_te){
							return e->data.addr == loc;
						});
					}
				}
			}
		}
		if(reg_info[insn_idx].resultf == FORWARD || 
			reg_info[insn_idx].resultf == BOTH) {
			count["forward"].running.inc();
		}
	}

	register_pending(pending_value);
	register_pending(retire_value);
	values->push_back(pending_value);
	values->push_back(retire_value);
}

void vec1dflow_t::process(pe_ready_value_tvalue) {
	TIME_VIOLATION_CHECK
	uint32_t insn_idx = value->data->idx;
	progress_map[insn_idx] = 0;
	window_start = (window_start + 1) % window_size;
	if(active_insn_offset > 0) active_insn_offset--;
	if(active_window_size > 0) active_window_size--;
	// std::cout << "Readying: " << std::hex << value->data->ws.pc;
	// std::cout << " " << active_window_size << std::endl;
	for(auto parent : parents.raw<vec_signal_handler_t *>()) {
		values->push_back(
			new vec_ready_value_t(parent.second, value->data, clock.get()));
		// Also check that all outstanding memory accesses completed
		if(active_window_size == 0) {
			empty = true;
			start = false;
			auto vec_retire_value = new vec_retire_value_t(
				parent.second, value->data, clock.get());
			if(promote_pending(vec_retire_value, [&](){
				return outstanding != 0;
			}) != nullptr); 
			else values->push_back(vec_retire_value);
		}
	}
}
