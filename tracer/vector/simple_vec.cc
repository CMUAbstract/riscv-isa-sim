#include "simple_vec.h"

#include "mem_event.h"
#include "vector_event.h"
#include "pending_event.h"

simple_vec_t::simple_vec_t(std::string _name, io::json _config, event_heap_t *_events)
	: vcu_t(_name, _config, _events) {
	state.resize(reg_count, 0);		
}

void simple_vec_t::process(vector_exec_event_t *event) {
	TIME_VIOLATION_CHECK
	events->push_back(new pe_exec_event_t(this, event->data, clock.get()));
	event->data->idx = state.size() + retired_idx;
	state.push_back(0);
}

void simple_vec_t::process(pe_exec_event_t *event) {
	TIME_VIOLATION_CHECK
	uint32_t idx = state[event->data->idx - retired_idx];
	pending_event_t *pending_event;
	if(idx + 1 == vl) {
		event->ready_gc = true;
		pending_event = new pending_event_t(this, 
			new pe_ready_event_t(this, event->data, clock.get()), clock.get() + 1);
	} else {
		event->ready_gc = false;
		pending_event = new pending_event_t(this, event, clock.get() + 1);
	}

	if(idx < event->data->ws.input.locs.size()) {
		auto input_loc = *std::next(event->data->ws.input.locs.begin(), idx);
		for(auto child : children.raw<ram_handler_t *>()) {
			events->push_back(
				new mem_read_event_t(child.second, input_loc, clock.get()));
			pending_event->add_dep<mem_ready_event_t *>(
				[input_loc](mem_ready_event_t *e){
				return e->data.addr == input_loc;
			});
		}
	}

	for(auto it : event->data->ws.input.vregs) {
		events->push_back(
			new vector_reg_read_event_t(this, {.reg=it, .idx=idx}, clock.get()));
		pending_event->add_dep<vector_reg_read_event_t *>(
			[reg_val=it, idx_val=idx](vector_reg_read_event_t *e){
				return e->data.reg == reg_val && e->data.idx == idx_val;
		});
	}

	if(idx < event->data->ws.output.locs.size()) {
		auto output_loc = *std::next(event->data->ws.output.locs.begin(), idx);
		for(auto child : children.raw<ram_handler_t *>()) {
			events->push_back(
				new mem_write_event_t(child.second, output_loc, clock.get()));
			pending_event->add_dep<mem_ready_event_t *>(
				[output_loc](mem_ready_event_t *e){
				return e->data.addr == output_loc;
			});
		}
	}

	uint32_t latency = 0;
	if(event->data->ws.input.vregs.size() > 0) latency = 1;
	for(auto it : event->data->ws.output.vregs) {
		events->push_back(
			new vector_reg_write_event_t(
				this, {.reg=it, .idx=idx}, clock.get() + latency));
		pending_event->add_dep<vector_reg_read_event_t *>(
			[reg_val=it, idx_val=idx](vector_reg_read_event_t *e){
				return e->data.reg == reg_val && e->data.idx == idx_val;
		});
	}

	state[event->data->idx]++; // increment index
	register_pending(pending_event);
	events->push_back(pending_event);
}

void simple_vec_t::process(pe_ready_event_t *event) {
	TIME_VIOLATION_CHECK
	for(auto parent : parents.raw<vector_signal_handler_t *>()) {
		events->push_back(
			new vector_ready_event_t(parent.second, event->data, clock.get()));
		events->push_back(
			new vector_retire_event_t(parent.second, event->data, clock.get()));
	}
	state.pop_front();
	retired_idx++;
}

void simple_vec_t::process(vector_reg_read_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
}

void simple_vec_t::process(vector_reg_write_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
}

void simple_vec_t::process(mem_ready_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
}

void simple_vec_t::process(mem_retire_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
}

void simple_vec_t::process(mem_match_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
}
