#include "main_mem.h"

#include "mem_event.h"

main_mem_t::main_mem_t(std::string _name, io::json _config, event_list_t *_events)
	: mem_t(_name, _config, _events) {
	JSON_CHECK(int, config["read_latency"], read_latency, 1);
	JSON_CHECK(int, config["write_latency"], write_latency, 1);
}

void main_mem_t::process(mem_read_event_t *event){
	TIME_VIOLATION_CHECK
	for(auto parent : parents) {
		events->push_back(
			new mem_ready_event_t(
				parent.second, event->data, clock.get() + read_latency, event));
	}
}

void main_mem_t::process(mem_write_event_t *event){
	TIME_VIOLATION_CHECK
	for(auto parent : parents) {
		events->push_back(
			new mem_ready_event_t(
				parent.second, event->data, clock.get() + write_latency, event));
	}
}
