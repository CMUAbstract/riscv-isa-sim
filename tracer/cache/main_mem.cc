#include "main_mem.h"

#include "mem_event.h"
#include "signal_event.h"
#include "pending_event.h"

main_mem_t::main_mem_t(std::string _name, io::json _config, event_list_t *_events)
	: ram_t(_name, _config, _events) {
	JSON_CHECK(int, config["read_latency"], read_latency);
	JSON_CHECK(int, config["write_latency"], write_latency);
	JSON_CHECK(int, config["ports"], ports);
	status["read"] = 0;
	status["write"] = 0;
}

void main_mem_t::process(mem_read_event_t *event){
	TIME_VIOLATION_CHECK
	if(status["write"] + status["read"] >= ports) { // Pending event promotion
		event->ready_gc = false;
		auto pending_event = new pending_event_t(this, 
			event, clock.get() + 1);
		pending_event->add_dependence([&](){ 
			return status["write"] + status["read"] < ports; 
		});
		register_pending(pending_event);
		events->push_back(pending_event);
		return;
	}
	reads.inc();
	// Increment write and also queue event to decrement write
	status["read"]++;
	auto pending_event = new pending_event_t(
		this, nullptr, clock.get() + read_latency);
	pending_event->add_fini([&](){ status["read"]--; });
	register_pending(pending_event);
	events->push_back(pending_event);
	for(auto parent : parents.raw<ram_t *>()) {
		events->push_back(
			new mem_insert_event_t(
				parent.second, event->data, clock.get() + read_latency));
	}
	for(auto parent : parents.raw<signal_handler_t *>()) {
		events->push_back(
			new ready_event_t(
				parent.second, event->data, clock.get() + read_latency));
	}
}

void main_mem_t::process(mem_write_event_t *event){
	TIME_VIOLATION_CHECK
	if(status["write"] + status["read"] >= ports) { // Pending event promotion
		event->ready_gc = false;
		auto pending_event = new pending_event_t(this, 
			event, clock.get() + 1);
		pending_event->add_dependence([&](){ 
			return status["write"] + status["read"] < ports; 
		});
		register_pending(pending_event);
		events->push_back(pending_event);
		return;
	}
	writes.inc();
	// Increment write and also queue event to decrement write
	status["write"]++;
	auto pending_event = new pending_event_t(
		this, nullptr, clock.get() + write_latency);
	pending_event->add_fini([&](){ status["write"]--; });
	register_pending(pending_event);
	events->push_back(pending_event);
	for(auto parent : parents.raw<ram_t *>()) {
		events->push_back(
			new mem_insert_event_t(
				parent.second, event->data, clock.get() + write_latency));
	}
	for(auto parent : parents.raw<signal_handler_t *>()) {
		events->push_back(
			new ready_event_t(
				parent.second, event->data, clock.get() + write_latency));
	}
}

void main_mem_t::process(pending_event_t *event) {
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
	remove_pending(event);
	if(event->data != nullptr) {
		event->data->ready_gc = true;
		event->data->cycle = clock.get();
		events->push_back(event->data);
	}
}
