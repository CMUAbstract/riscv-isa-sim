#include "vecflow.h"

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
}
void vecflow_t::process(vector_exec_event_t *event) {
	TIME_VIOLATION_CHECK
	if(promote_pending(event, [&](){
		return !(active_window_size < window_size);
	}) != nullptr) return;
	event->data->idx = idx; 
	idx = (idx + 1) % window_size;
	active_window_size++;
	if(active_window_size == window_size) vcu_t::set_core_stage("exec", true);
	events->push_back(new pe_exec_event_t(this, event->data, clock.get()));
}

void vecflow_t::process(pe_exec_event_t *event) {
	TIME_VIOLATION_CHECK

}

void vecflow_t::process(pe_ready_event_t *event) {
	TIME_VIOLATION_CHECK
	for(auto parent : parents.raw<vector_signal_handler_t *>()) {
		events->push_back(
			new vector_ready_event_t(parent.second, event->data, clock.get()));
		if(active_window_size == 0) {
			events->push_back(
				new vector_retire_event_t(parent.second, event->data, clock.get()));
		}
	}
}