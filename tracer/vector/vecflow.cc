#include "vecflow.h"

#include <algorithm>
#include <iterator>

#include "core.h"
#include "mem_event.h"
#include "vector_event.h"
#include "pending_event.h"

// Throw vector_retire when window has been cleared
// Throw vector_ready when place in window has freed up
vecflow_t::vecflow_t(std::string _name, io::json _config, event_heap_t *_events)
	: vcu_t(_name, _config, _events) {
	JSON_CHECK(int, config["window_size"], window_size, 1);
	indices.resize(window_size, 0);
	active_lanes.resize(window_size, 0);
	outputs.resize(window_size);
}
void vecflow_t::process(vector_exec_event_t *event) {
	TIME_VIOLATION_CHECK
	if(promote_pending(event, [&](){
		return !(active_window_size < window_size);
	}) != nullptr) return;
	event->data->idx = idx;
	uint32_t fr = event->data->insn.fr();
	if(fr == VCU_FORWARD || fr == VCU_FORWARD_REG_WRITE) {
		for(auto reg : event->data->ws.output.vregs)
			outputs[idx].insert(reg);
	}
	empty = false;
	idx = (idx + 1) % window_size;
	active_window_size++;
	if(active_window_size == window_size) vcu_t::set_core_stage("exec", true);
	events->push_back(new pe_exec_event_t(this, event->data, clock.get()));
}

void vecflow_t::process(pe_exec_event_t *event) {
	TIME_VIOLATION_CHECK

	uint32_t insn_idx = event->data->idx;
	if(promote_pending(event, [&](){
		return !(active_lanes[insn_idx] < lanes);
	}) != nullptr) return;

	uint32_t remaining = vl - indices[insn_idx];
	uint32_t local_idx = indices[insn_idx];
	if(remaining > lanes) remaining = lanes;
	pending_event_t *pending_event;
	if(vl - local_idx > lanes) {
		pending_event = new pending_event_t(this, 
			new pe_exec_event_t(this, event->data), clock.get() + 1);
		pending_event->add_fini([&, insn_idx, remaining](){ 
			active_lanes[insn_idx] -= remaining; 
		});
	} else {
		pending_event = new pending_event_t(this, 
			new pe_ready_event_t(this, event->data), clock.get() + 1);
		pending_event->add_fini([&, insn_idx, remaining](){ 
			active_lanes[insn_idx] -= remaining; 
			vcu_t::set_core_stage("exec", false);
		});
	}

	active_lanes[insn_idx] += remaining;
	indices[insn_idx] += remaining;

	// Input locations
	if(local_idx < event->data->ws.input.locs.size()) {
		auto it = std::next(event->data->ws.input.locs.begin(), local_idx);
		auto end = std::next(event->data->ws.input.locs.begin(), local_idx + remaining);
		while(it != end) {
			for(auto child : children.raw<ram_handler_t *>()) {
				auto loc = *it;
				events->push_back(
					new mem_read_event_t(child.second, loc, clock.get()));
				pending_event->add_dep<mem_ready_event_t *>(
					[loc](mem_ready_event_t *e){
					return e->data.addr == loc;
				});
			}
			++it;
		}
	}

	// Output Locations
	if(local_idx < event->data->ws.output.locs.size()) {
		auto it = std::next(event->data->ws.input.locs.begin(), local_idx);
		auto end = std::next(event->data->ws.input.locs.begin(), local_idx + remaining);
		while(it != end) {
			for(auto child : children.raw<ram_handler_t *>()) {
				auto loc = *it;
				events->push_back(
					new mem_write_event_t(child.second, loc, clock.get()));
				pending_event->add_dep<mem_ready_event_t *>(
					[loc](mem_ready_event_t *e){
					return e->data.addr == loc;
				});
			}
			++it;
		}
	}

	// Input registers
	uint32_t latency = 0;
	auto reg_set = get_reg_set(event->data);
	for(auto it : event->data->ws.input.vregs) {
		for(auto loc = local_idx; loc < local_idx + remaining; loc++) {
			if(reg_set.size() != 0 && reg_set.find(it) != reg_set.end()) {
				latency = 1;
				events->push_back(
					new vector_reg_read_event_t(
						this, {.reg=it, .idx=loc}, clock.get()));
				pending_event->add_dep<vector_reg_read_event_t *>(
					[it, loc](vector_reg_read_event_t *e){
						return e->data.reg == it && e->data.idx == loc;
				});
				continue;
			}
			events->push_back(
				new pipeline_reg_read_event_t(
					this, {.reg=0, .vreg=it, .idx=loc}, clock.get()));
			pending_event->add_dep<pipeline_reg_read_event_t *>(
				[it, loc](pipeline_reg_read_event_t *e){
					return e->data.vreg == it && e->data.idx == loc;
			});
		}
	}

	// Output registers
	uint32_t fr = event->data->insn.fr();
	if(fr == VCU_REG_WRITE || fr == VCU_FLUSH) {
		for(auto it : event->data->ws.output.vregs) {
			for(auto loc = local_idx; loc < local_idx + remaining; loc++) {
				events->push_back(
					new vector_reg_write_event_t(
						this, {.reg=it, .idx=loc}, clock.get() + latency));
				pending_event->add_dep<vector_reg_write_event_t *>(
					[it, loc](vector_reg_write_event_t *e){
						return e->data.reg == it && e->data.idx == loc;
				});
			}
		}
	} else if(fr == VCU_FORWARD) {
		for(auto it : event->data->ws.output.vregs) {
			for(auto loc = local_idx; loc < local_idx + remaining; loc++) {
				events->push_back(
					new pipeline_reg_write_event_t(
						this, {.reg=0, .vreg=it, .idx=loc}, clock.get() + latency));
				pending_event->add_dep<pipeline_reg_read_event_t *>(
					[it, loc](pipeline_reg_read_event_t *e){
						return e->data.vreg == it && e->data.idx == loc;
				});
			}
		}
	} else if(fr == VCU_FORWARD_REG_WRITE) {
		for(auto it : event->data->ws.output.vregs) {
			for(auto loc = local_idx; loc < local_idx + remaining; loc++) {
				events->push_back(
					new vector_reg_write_event_t(
						this, {.reg=it, .idx=loc}, clock.get() + latency));
				pending_event->add_dep<vector_reg_write_event_t *>(
					[it, loc](vector_reg_write_event_t *e){
						return e->data.reg == it && e->data.idx == loc;
				});
				events->push_back(
					new pipeline_reg_write_event_t(
						this, {.reg=0, .vreg=it, .idx=loc}, clock.get() + latency));
				pending_event->add_dep<pipeline_reg_read_event_t *>(
					[it, loc](pipeline_reg_read_event_t *e){
						return e->data.vreg == it && e->data.idx == loc;
				});
			}
		}
	}
	register_pending(pending_event);
	events->push_back(pending_event);
}

void vecflow_t::process(pe_ready_event_t *event) {
	TIME_VIOLATION_CHECK
	uint32_t insn_idx = event->data->idx;
	active_lanes[insn_idx] = 0;
	indices[insn_idx] = 0;
	outputs[insn_idx].clear();
	if(active_window_size > 0) active_window_size--;
	for(auto parent : parents.raw<vector_signal_handler_t *>()) {
		events->push_back(
			new vector_ready_event_t(parent.second, event->data, clock.get()));
		if(active_window_size == 0) {
			empty = true;
			events->push_back(
				new vector_retire_event_t(parent.second, event->data, clock.get()));
		}
	}
}

void vecflow_t::process(pipeline_reg_read_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
}

void vecflow_t::process(pipeline_reg_write_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
}

std::set<uint32_t> vecflow_t::get_reg_set(hstd::shared_ptr<timed_insn_t> timed_insn) {
	std::set<uint32_t> unaccounted_inputs = timed_insn->ws.input.vregs;
	for(uint32_t i = 1; i < active_window_size; i++) {
		int32_t cur_idx = timed_insn->idx - i;
		if(cur_idx < 0) cur_idx = window_size + cur_idx;
		std::set<uint32_t> diff;
		std::set_difference(unaccounted_inputs.begin(), unaccounted_inputs.end(),
			outputs[cur_idx].begin(), outputs[cur_idx].end(), 
			std::inserter(diff, diff.begin()));
		unaccounted_inputs = diff;
	}
	return unaccounted_inputs;
}