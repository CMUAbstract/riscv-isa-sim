#include "simple_vec.h"

#include "mem_event.h"
#include "signal_event.h"
#include "vector_event.h"
#include "pending_event.h"

simple_vec_t::simple_vec_t(std::string _name, io::json _config, event_heap_t *_events)
	: vcu_t(_name, _config, _events) {}

void simple_vec_t::process(vector_exec_event_t *event) {
	TIME_VIOLATION_CHECK
	events->push_back(new pe_exec_event_t(this, event->data, clock.get()));
}

void simple_vec_t::process(pe_exec_event_t *event) {
	TIME_VIOLATION_CHECK
	bool finish_op = false;
	for(auto it : event->data->ws.input.vregs) {
		if(state[it] + 1 == vl) {
			finish_op = true;
			break;
		}
	}

	pending_event_t *pending_event;
	if(finish_op) {
		event->ready_gc = true;
		pending_event = new pending_event_t(this, 
			new pe_ready_event_t(this, event->data, clock.get()), clock.get() + 1);
	} else {
		event->ready_gc = false;
		pending_event = new pending_event_t(this, event, clock.get() + 1);
	}

	for(auto it : event->data->ws.input.locs) {
		for(auto child : children.raw<ram_handler_t *>()) {
			events->push_back(new mem_read_event_t(child.second, it, clock.get()));
			pending_event->add_dependence<ready_event_t *>(
				[it](ready_event_t *e){
				return e->data == it;
			});
		}
	}

	for(auto it : event->data->ws.input.vregs) {
		events->push_back(
			new vector_reg_read_event_t(this, {.reg=it, .idx=state[it]}, clock.get()));
		pending_event->add_dependence<vector_reg_read_event_t *>(
			[reg=it, idx=state[it]](vector_reg_read_event_t *e){
			return e->data.reg == reg && e->data.idx == idx;
		});
		state[it]++;
	}

	for(auto it : event->data->ws.output.locs) {
		for(auto child : children.raw<ram_handler_t *>()) {
			events->push_back(new mem_write_event_t(child.second, it, clock.get()));
			pending_event->add_dependence<ready_event_t *>(
				[it](ready_event_t *e){
				return e->data == it;
			});
		}
	}

	for(auto it : event->data->ws.output.vregs) {
		events->push_back(
			new vector_reg_write_event_t(
				this, {.reg=it, .idx=state[it]}, clock.get() + 1));
		pending_event->add_dependence<vector_reg_read_event_t *>(
			[reg=it, idx=state[it]](vector_reg_read_event_t *e){
			return e->data.reg == reg && e->data.idx == idx;
		});
		state[it]++;
	}

	register_pending(pending_event);
	events->push_back(pending_event);
}

void simple_vec_t::process(pe_ready_event_t *event) {
	TIME_VIOLATION_CHECK
	for(auto it : event->data->ws.input.vregs) state[it] = 0;
	for(auto it : event->data->ws.output.vregs) state[it] = 0;
	for(auto parent : parents.raw<vector_signal_handler_t *>()) {
		events->push_back(
			new vector_ready_event_t(parent.second, event->data, clock.get()));
		events->push_back(
			new vector_retire_event_t(parent.second, event->data, clock.get()));
	}
}

void simple_vec_t::process(pending_event_t *event) {
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

void simple_vec_t::process(vector_reg_read_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
}

void simple_vec_t::process(vector_reg_write_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
}

void simple_vec_t::process(ready_event_t *event){
	TIME_VIOLATION_CHECK
	check_pending(event);
}

void simple_vec_t::process(stall_event_t *event){
	TIME_VIOLATION_CHECK
}
