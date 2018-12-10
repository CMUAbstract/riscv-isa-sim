#include "vec1d.h"

#include "mem_event.h"
#include "vector_event.h"
#include "pending_event.h"

vec1d_t::vec1d_t(std::string _name, io::json _config, event_heap_t *_events)
	: vcu_t(_name, _config, _events) {
	JSON_CHECK(int, config["lanes"], lanes, 1);
	assert_msg(lanes > 0, "Must have at least 1 lane");
}

void vec1d_t::process(vector_exec_event_t *event) {
	TIME_VIOLATION_CHECK
	events->push_back(new pe_exec_event_t(this, event->data, clock.get()));
	event->data->idx = state.size() + retired_idx;
	state.push_back(0);
}

void vec1d_t::process(pe_exec_event_t *event) {
	TIME_VIOLATION_CHECK
	uint32_t idx = state[event->data->idx - retired_idx];
	pending_event_t *pending_event;
	if(idx + lanes >= vl) {
		event->ready_gc = true;
		pending_event = new pending_event_t(this, 
			new pe_ready_event_t(this, event->data, clock.get()), clock.get() + 1);
	} else {
		event->ready_gc = false;
		pending_event = new pending_event_t(this, event, clock.get() + 1);
	}
	uint32_t remaining = (lanes > vl - idx) ? vl - idx : lanes;
#if 0
	std::cout << "idx: " << idx << " vl: " << vl << " lanes: "<< lanes;
	std::cout << " remaining: " << remaining << std::endl;
#endif

	if(idx < event->data->ws.input.locs.size()) {
		auto it = std::next(event->data->ws.input.locs.begin(), idx);
		auto end = std::next(event->data->ws.input.locs.begin(), idx + remaining);
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

	for(auto it : event->data->ws.input.vregs) {
		for(auto loc = idx; loc < idx + remaining; loc++) {
			events->push_back(
				new vector_reg_read_event_t(this, {.reg=it, .idx=loc}, clock.get()));
			pending_event->add_dep<vector_reg_read_event_t *>(
				[it, loc](vector_reg_read_event_t *e){
					return e->data.reg == it && e->data.idx == loc;
			});
		}
	}

	if(idx < event->data->ws.output.locs.size()) {
		auto it = std::next(event->data->ws.input.locs.begin(), idx);
		auto end = std::next(event->data->ws.input.locs.begin(), idx + remaining);
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

	uint32_t latency = 0;
	if(event->data->ws.input.vregs.size() > 0) latency = 1;
	for(auto it : event->data->ws.output.vregs) {
		for(auto loc = idx; loc < idx + remaining; loc++) {
			events->push_back(
				new vector_reg_write_event_t(
					this, {.reg=it, .idx=loc}, clock.get() + latency));
			pending_event->add_dep<vector_reg_write_event_t *>(
				[it, loc](vector_reg_write_event_t *e){
					return e->data.reg == it && e->data.idx == loc;
			});
		}
	}

	state[event->data->idx - retired_idx] += remaining; // increment index
	register_pending(pending_event);
	events->push_back(pending_event);
}

void vec1d_t::process(pe_ready_event_t *event) {
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

void vec1d_t::process(vector_reg_read_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
}

void vec1d_t::process(vector_reg_write_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
}

void vec1d_t::process(mem_ready_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
}

void vec1d_t::process(mem_retire_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
}

void vec1d_t::process(mem_match_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
}
