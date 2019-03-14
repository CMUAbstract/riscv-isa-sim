#include "vec1dflow.h"

#include "mem_event.h"
#include "vector_event.h"
#include "pending_event.h"

vec1dflow_t::vec1dflow_t(std::string _name, io::json _config, 
	event_heap_t *_events) : vcu_t(_name, _config, _events) {
	JSON_CHECK(int, config["window_size"], window_size);
	assert_msg(window_size > 0, "Window size must be greater than zero");
	JSON_CHECK(bool, config["src_forwarding"], src_forwarding);
	reg_map.resize(reg_count, false);
	kill_map.resize(reg_count, false);
	src_map.resize(reg_count, false);
	progress_map.resize(window_size, 0);
}

io::json vec1dflow_t::to_json() const {
	return vcu_t::to_json();
}

void vec1dflow_t::reset(reset_level_t level) {
	vcu_t::reset(level);
	start = false;

	idx = 0;
	window_start = 0;
	active_insn_offset = 0;
	active_window_size = 0;

	std::fill(reg_map.begin(), reg_map.end(), false);
	std::fill(kill_map.begin(), kill_map.end(), false);
	std::fill(src_map.begin(), src_map.end(), false);
	std::fill(progress_map.begin(), progress_map.end(), 0);
}

void vec1dflow_t::process(vector_exec_event_t *event) {
	TIME_VIOLATION_CHECK;
	if(promote_pending(event, [&](){
		return !(active_window_size < window_size);
	}) != nullptr) return;
	event->data->idx = idx;
	idx = (idx + 1) % window_size;
	empty = false;
	active_window_size++;
	if(active_window_size == window_size) {
		vcu_t::set_core_stage("exec", true);
		events->push_back(new vector_start_event_t(this, false, clock.get()));
	}
	events->push_back(new pe_exec_event_t(this, event->data, clock.get()));
}

void vec1dflow_t::process(vector_start_event_t *event) {
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
			pending_event->add_dep<vector_start_event_t *>(
				[](vector_start_event_t *e) { return true; });
		}
	} else {
		pending_event = new pending_event_t(this, 
			new pe_ready_event_t(this, event->data), clock.get() + 1);
		pending_event->add_fini([&, insn_idx](){ 
			progress_map[insn_idx] = 0; 
			vcu_t::set_core_stage("exec", false);
		});
	}

	auto retire_event = new pending_event_t(this, nullptr, clock.get() + 1);
	retire_event->add_fini([&](){ outstanding = 0; });
	outstanding = 1;

	// Input locations
	if(cur_progress < event->data->ws.input.locs.size()) {
		auto it = std::next(event->data->ws.input.locs.begin(), cur_progress);
		auto end = std::next(
			event->data->ws.input.locs.begin(), cur_progress + work);
		while(it != end) {
			for(auto child : children.raw<ram_handler_t *>()) {
				auto loc = *it;
				events->push_back(
					new mem_read_event_t(child.second, loc, clock.get()));
				pending_event->add_dep<mem_ready_event_t *>(
					[loc](mem_ready_event_t *e){
					return e->data.addr == loc;
				});
				retire_event->add_dep<mem_retire_event_t *>(
					[loc](mem_retire_event_t *e){
					return e->data.addr == loc;
				});
			}
			++it;
		}
	}

	// Output locations
	if(cur_progress < event->data->ws.output.locs.size()) {
		auto it = std::next(event->data->ws.output.locs.begin(), cur_progress);
		auto end = std::next(
			event->data->ws.output.locs.begin(), cur_progress + work);
		while(it != end) {
			for(auto child : children.raw<ram_handler_t *>()) {
				auto loc = *it;
				events->push_back(
					new mem_read_event_t(child.second, loc, clock.get()));
				pending_event->add_dep<mem_ready_event_t *>(
					[loc](mem_ready_event_t *e){
					return e->data.addr == loc;
				});
				retire_event->add_dep<mem_retire_event_t *>(
					[loc](mem_retire_event_t *e){
					return e->data.addr == loc;
				});
			}
			++it;
		}
	}

	// Input registers
	for(auto it : event->data->ws.input.vregs) {
		uint8_t reg = strip_killed(it);
		if(!reg_map[reg]) {
			events->push_back(new vector_reg_read_event_t(
				this, {.reg=reg, .idx=0}, clock.get()));
			pending_event->add_dep<vector_reg_read_event_t *>(
				[reg](vector_reg_read_event_t *e){
					return e->data.reg == reg;
			});
			if(src_forwarding) {
				reg_map[reg] = true;
				src_map[reg] = true;
			}
		} else {
			kill_map[reg] = check_killed(it);
		}
	}

	// Scalar register reads
	for(auto it : event->data->ws.input.regs) {
		events->push_back(new reg_read_event_t(core, it, clock.get()));
		pending_event->add_dep<reg_read_event_t *>([it](reg_read_event_t *e){
			return e->data == it;
		});
	}

	// Output registers
	for(auto it : event->data->ws.output.vregs) {
		uint8_t reg = strip_killed(it);
		events->push_back(new vector_reg_write_event_t(
			this, {.reg=reg, .idx=0}, clock.get()));
		pending_event->add_dep<vector_reg_write_event_t *>(
			[reg](vector_reg_write_event_t *e){
				return e->data.reg == reg;
		});
		reg_map[reg] = true;
		src_map[reg] = false;
	}

	if(active_insn_offset + 1 == active_window_size && start) {
		active_insn_offset = 0;
		// Issue writes for alive registers
		uint16_t reg = 0;
		for(auto reg_state : reg_map) {
			if(reg_state && !kill_map[reg] && !src_map[reg]) {
				events->push_back(new vector_reg_write_event_t(
				this, {.reg=reg, .idx=0}, clock.get()));
				pending_event->add_dep<vector_reg_write_event_t *>(
					[reg](vector_reg_write_event_t *e){
						return e->data.reg == reg;
				});
			}
			reg++;
		}
	} else active_insn_offset++;

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
	for(auto parent : parents.raw<vector_signal_handler_t *>()) {
		events->push_back(
			new vector_ready_event_t(parent.second, event->data, clock.get()));
		// Also check that all outstanding memory accesses completed
		if(active_window_size == 0) {
			empty = true;
			start = false;
			auto vector_retire_event = new vector_retire_event_t(
				parent.second, event->data, clock.get());
			if(promote_pending(vector_retire_event, [&](){
				return outstanding != 0;
			}) != nullptr); 
			else events->push_back(vector_retire_event);
		}
	}
}