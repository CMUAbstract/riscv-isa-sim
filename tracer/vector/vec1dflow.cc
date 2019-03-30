#include "vec1dflow.h"

#include "mem_event.h"
#include "vector_event.h"
#include "pending_event.h"

vec1dflow_t::vec1dflow_t(std::string _name, io::json _config, 
	event_heap_t *_events) : vcu_t(_name, _config, _events) {
	JSON_CHECK(int, config["window_size"], window_size);
	assert_msg(window_size > 0, "Window size must be greater than zero");
	JSON_CHECK(bool, config["src_forwarding"], src_forwarding);

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

void vec1dflow_t::process(vec_issue_event_t *event) {
	TIME_VIOLATION_CHECK;
	if(promote_pending(event, [&](){
		return !(active_window_size < window_size);
	}) != nullptr) return;

	event->data->idx = idx;

	reg_info[idx].op1f = false;
	reg_info[idx].op2f = false;
	reg_info[idx].resultf = REG;
	reg_info[idx].op1 = -1;
	reg_info[idx].op2 = -1;
	reg_info[idx].result = -1;
	if(event->data->ws.output.vregs.size() > 0) {
		reg_info[idx].result = strip_killed(event->data->insn.rd());
	}

	if(event->data->ws.input.vregs.size() >= 1) {
		reg_info[idx].op1 = strip_killed(event->data->insn.rs1());
		for(uint16_t i = 1; i <= active_window_size; i++) {
			uint16_t window_idx = (window_size + window_start + active_window_size - i);
			window_idx %= window_size;
			if(reg_info[window_idx].result == reg_info[idx].op1) {
				reg_info[window_idx].resultf = BOTH;
				reg_info[idx].op1f = true;
				if(check_killed(event->data->insn.rs1())) {
					reg_info[window_idx].resultf = FORWARD;
				}
				break;
			} else if(src_forwarding) {
				assert_msg(1 == 0, "Source forwarding not yet implemented");
			}
		}
	}

	if(event->data->ws.input.vregs.size() >= 2) {
		reg_info[idx].op2 = strip_killed(event->data->insn.rs2());
		for(uint16_t i = 1; i <= active_window_size; i++) {
			uint16_t window_idx = (window_size + window_start + active_window_size - i);
			window_idx %= window_size;
			if(reg_info[window_idx].result == reg_info[idx].op2) {
				reg_info[window_idx].resultf = BOTH;
				reg_info[idx].op2f = true;
				if(check_killed(event->data->insn.rs2())) {
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
	// std::cout << "Issuing: " << std::hex << event->data->ws.pc;
	// std::cout << " " << active_window_size << std::endl;
	if(active_window_size == window_size || check_split(event->data->opc)) {
		events->push_back(new vec_start_event_t(this, false, clock.get()));
	} else { // Issue ready signals
		for(auto parent : parents.raw<vec_signal_handler_t *>()) {
			events->push_back(
				new vec_ready_event_t(parent.second, event->data, clock.get()));
		}
	}

	auto pending_event = new pending_event_t(this,
		new pe_exec_event_t(this, event->data), clock.get());
	pending_event->add_dep<vec_start_event_t *>([](vec_start_event_t *event) {
		return true;
	});

	register_pending(pending_event);
	events->push_back(pending_event);
}

void vec1dflow_t::process(vec_start_event_t *event) {
	vcu_t::process(event);
	start = true;
	if(active_insn_offset == active_window_size) active_insn_offset = 0;
}

void vec1dflow_t::process(pe_exec_event_t *event) {
	TIME_VIOLATION_CHECK;
	uint16_t insn_idx = event->data->idx;
	if(promote_pending(event, [&, insn_idx]() {
		uint16_t cur_idx = (window_start + active_insn_offset) % window_size;
		return !(cur_idx == insn_idx && outstanding == 0);
	}) != nullptr) return;

	uint16_t work = vl - progress_map[insn_idx];
	uint16_t cur_progress = progress_map[insn_idx];
	pending_event_t *pending_event;
	if(work > lanes) {
		work = lanes;
		pending_event = new pending_event_t(this, 
			new pe_exec_event_t(this, event->data), clock.get() + 1);
		pending_event->add_fini([&, insn_idx, work](){ 
			progress_map[insn_idx] += work;
		});
		if(!start) {
			pending_event->add_dep<vec_start_event_t *>(
				[](vec_start_event_t *e) { return true; });
		}
	} else {
		pending_event = new pending_event_t(this, 
			new pe_ready_event_t(this, event->data), clock.get() + 1);
		pending_event->add_fini([&, insn_idx](){ 
			progress_map[insn_idx] = 0; 
		});
	}

	auto retire_event = new pending_event_t(this, nullptr, clock.get() + 1);
	retire_event->add_fini([&](){ outstanding = 0; });
	outstanding = 1;

	if(check_mul(event->data->opc)) {
		count["mul"].running.inc();
	} else {
		count["alu"].running.inc();
	}

	// Input locations
	if(cur_progress < event->data->ws.input.locs.size()) {
		for(auto child : children.raw<ram_t *>()) {
			
			auto it = std::next(
				event->data->ws.input.locs.begin(), cur_progress);
			auto end = std::next(
				event->data->ws.input.locs.begin(), cur_progress + work);
			std::set<addr_t> locs;
			addr_t line_mask = ~(child.second->get_line_size() - 1);
			while(it != end) {
				locs.insert(*it & line_mask);
				++it;
			}

			for(auto loc : locs) {
				events->push_back(
					new mem_read_event_t(child.second, loc, clock.get()));
				pending_event->add_dep<mem_retire_event_t *>(
					[loc](mem_retire_event_t *e){
					return e->data.addr == loc;
				});
				retire_event->add_dep<mem_retire_event_t *>(
					[loc](mem_retire_event_t *e){
					return e->data.addr == loc;
				});
			}
		}
	}

	// Output locations
	if(cur_progress < event->data->ws.output.locs.size()) {
		for(auto child : children.raw<ram_t *>()) {
			
			auto it = std::next(
				event->data->ws.output.locs.begin(), cur_progress);
			auto end = std::next(
				event->data->ws.output.locs.begin(), cur_progress + work);
			std::set<addr_t> locs;
			addr_t line_mask = ~(child.second->get_line_size() - 1);
			while(it != end) {
				locs.insert(*it & line_mask);
				++it;
			}

			for(auto loc : locs) {
				events->push_back(
					new mem_write_event_t(child.second, loc, clock.get()));
				pending_event->add_dep<mem_retire_event_t *>(
					[loc](mem_retire_event_t *e){
					return e->data.addr == loc;
				});
				retire_event->add_dep<mem_retire_event_t *>(
					[loc](mem_retire_event_t *e){
					return e->data.addr == loc;
				});
			}
		}
	}

	// // Input registers
	if(event->data->ws.input.vregs.size() >= 1) {
		uint8_t reg = reg_info[insn_idx].op1;
#ifndef CACHE_VRF
		if(!reg_info[insn_idx].op1f) {
			for(uint16_t i = cur_progress; i < cur_progress + work; i++) {
				events->push_back(new vec_reg_read_event_t(
					this, {.reg=reg, .idx=i}, clock.get()));
				pending_event->add_dep<vec_reg_read_event_t *>(
					[reg, i](vec_reg_read_event_t *e){
						return e->data.reg == reg && e->data.idx == i;
				});
			}
		}
#else
		if(!reg_info[insn_idx].op1f) {
			for(auto child : children.raw<ram_t *>()) {
				std::set<addr_t> locs;
				addr_t line_mask = ~(child.second->get_line_size() - 1);
				for(uint16_t i = cur_progress; i < cur_progress + work; i++) {
					locs.insert(((reg << 4) | i) & line_mask);
				}
				for(auto loc : locs) {
					events->push_back(
						new mem_read_event_t(child.second, loc, clock.get()));
					pending_event->add_dep<mem_retire_event_t *>(
						[loc](mem_retire_event_t *e){
						return e->data.addr == loc;
					});
					retire_event->add_dep<mem_retire_event_t *>(
						[loc](mem_retire_event_t *e){
						return e->data.addr == loc;
					});
				}
			}
		} 
#endif
	}
	if(event->data->ws.input.vregs.size() >= 2) {
		uint8_t reg = reg_info[insn_idx].op2;
#ifndef CACHE_VRF
		if(!reg_info[insn_idx].op2f) {
			for(uint16_t i = cur_progress; i < cur_progress + work; i++) {
				events->push_back(new vec_reg_read_event_t(
					this, {.reg=reg, .idx=i}, clock.get()));
				pending_event->add_dep<vec_reg_read_event_t *>(
					[reg, i](vec_reg_read_event_t *e){
						return e->data.reg == reg && e->data.idx == i;
				});
			}
		}
#else
		if(!reg_info[insn_idx].op2f) {
			for(auto child : children.raw<ram_t *>()) {
				std::set<addr_t> locs;
				addr_t line_mask = ~(child.second->get_line_size() - 1);
				for(uint16_t i = cur_progress; i < cur_progress + work; i++) {
					locs.insert(((reg << 4) | i) & line_mask);
				}
				for(auto loc : locs) {
					events->push_back(
						new mem_read_event_t(child.second, loc, clock.get()));
					pending_event->add_dep<mem_retire_event_t *>(
						[loc](mem_retire_event_t *e){
						return e->data.addr == loc;
					});
					retire_event->add_dep<mem_retire_event_t *>(
						[loc](mem_retire_event_t *e){
						return e->data.addr == loc;
					});
				}
			}
		}
#endif
	}

	// Output registers
	if(event->data->ws.output.vregs.size() > 0) {
		uint8_t reg = reg_info[insn_idx].result;
#ifndef CACHE_VRF
		if(reg_info[insn_idx].resultf == REG ||
				reg_info[insn_idx].resultf == BOTH) {
			for(uint16_t i = cur_progress; i < cur_progress + work; i++) {
				events->push_back(new vec_reg_write_event_t(
					this, {.reg=reg, .idx=i}, clock.get()));
				pending_event->add_dep<vec_reg_write_event_t *>(
					[reg, i](vec_reg_write_event_t *e){
						return e->data.reg == reg && e->data.idx == i;
				});
			}
		}
#else
		if(reg_info[insn_idx].resultf == REG ||
				reg_info[insn_idx].resultf == BOTH) {
			for(auto child : children.raw<ram_t *>()) {
				std::set<addr_t> locs;
				addr_t line_mask = ~(child.second->get_line_size() - 1);
				for(uint16_t i = cur_progress; i < cur_progress + work; i++) {
					locs.insert(((reg << 4) | i) & line_mask);
				}
				for(auto loc : locs) {
					events->push_back(
						new mem_write_event_t(child.second, loc, clock.get()));
					pending_event->add_dep<mem_retire_event_t *>(
						[loc](mem_retire_event_t *e){
						return e->data.addr == loc;
					});
					retire_event->add_dep<mem_retire_event_t *>(
						[loc](mem_retire_event_t *e){
						return e->data.addr == loc;
					});
				}
			}
		}
#endif
		if(reg_info[insn_idx].resultf == FORWARD || 
			reg_info[insn_idx].resultf == BOTH) {
			count["forward"].running.inc();
		}
	}

	register_pending(pending_event);
	register_pending(retire_event);
	events->push_back(pending_event);
	events->push_back(retire_event);
}

void vec1dflow_t::process(pe_ready_event_t *event) {
	TIME_VIOLATION_CHECK
	uint32_t insn_idx = event->data->idx;
	progress_map[insn_idx] = 0;
	window_start = (window_start + 1) % window_size;
	if(active_insn_offset > 0) active_insn_offset--;
	if(active_window_size > 0) active_window_size--;
	// std::cout << "Readying: " << std::hex << event->data->ws.pc;
	// std::cout << " " << active_window_size << std::endl;
	for(auto parent : parents.raw<vec_signal_handler_t *>()) {
		events->push_back(
			new vec_ready_event_t(parent.second, event->data, clock.get()));
		// Also check that all outstanding memory accesses completed
		if(active_window_size == 0) {
			empty = true;
			start = false;
			auto vec_retire_event = new vec_retire_event_t(
				parent.second, event->data, clock.get());
			if(promote_pending(vec_retire_event, [&](){
				return outstanding != 0;
			}) != nullptr); 
			else events->push_back(vec_retire_event);
		}
	}
}
