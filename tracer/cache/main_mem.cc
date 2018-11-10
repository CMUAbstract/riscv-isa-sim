#include "main_mem.h"

#include "mem_event.h"
#include "signal_event.h"

main_ram_t::main_ram_t(std::string _name, io::json _config, event_list_t *_events)
	: ram_t(_name, _config, _events) {
	JSON_CHECK(int, config["read_latency"], read_latency, 1);
	JSON_CHECK(int, config["write_latency"], write_latency, 1);
}

void main_ram_t::process(mem_read_event_t *event){
	TIME_VIOLATION_CHECK
	reads.inc();
	for(auto parent : parents) {
		auto m = dynamic_cast<ram_t *>(parent.second);
		if(m != nullptr) {
			events->push_back(
				new mem_insert_event_t(
					m, event->data, clock.get() + read_latency, event));
		}
		auto p = dynamic_cast<signal_handler_t *>(parent.second);
		if(p == nullptr) continue;
		events->push_back(
			new ready_event_t(
				p, event->data, clock.get() + read_latency, event));
	}
}

void main_ram_t::process(mem_write_event_t *event){
	TIME_VIOLATION_CHECK
	writes.inc();
	for(auto parent : parents) {
		auto m = dynamic_cast<ram_t *>(parent.second);
		if(m != nullptr) {
			events->push_back(
				new mem_insert_event_t(
					m, event->data, clock.get() + read_latency, event));
		}
		auto p = dynamic_cast<signal_handler_t *>(parent.second);
		if(p == nullptr) continue;
		events->push_back(
			new ready_event_t(
				p, event->data, clock.get() + write_latency, event));
	}
}
