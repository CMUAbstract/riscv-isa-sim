#include "main_mem.h"

#include "mem_event.h"
#include "pending_event.h"

main_mem_t::main_mem_t(std::string _name, io::json _config, event_heap_t *_events)
	: ram_t(_name, _config, _events) {}

void main_mem_t::process(mem_read_event_t *event){
	TIME_VIOLATION_CHECK

	auto bank = get_bank(event->data.addr);
	if(promote_pending(event, [&, bank](){
		if(banks[bank].readers < read_ports_per_bank &&
			banks[bank].total() < ports_per_bank) return false; 
		return true;
	}) != nullptr) {
		// assert_msg(banks[bank].readerq <= load_buf_size, 
			// "%s (%lu): Reader queue overflow", name.c_str(), clock.get());
		banks[bank].readerq++;
		if(banks[bank].readerq <= load_buf_size) {
			for(auto parent : parents.raw<ram_signal_handler_t *>()) {
				events->push_back(
					new mem_ready_event_t(
						parent.second, event->data, clock.get() + 1));
			}
		}
		return;
	}

	reads.inc();

	// Increment readers
	banks[bank].readers++;
	if(banks[bank].readerq > 0) banks[bank].readerq--;
	auto pending_event = new pending_event_t(
		this, nullptr, clock.get() + read_latency);
	pending_event->add_fini([&, bank](){ banks[bank].readers--; });
	register_pending(pending_event);
	events->push_back(pending_event);

	for(auto parent : parents.raw<ram_signal_handler_t *>()) {
		events->push_back(
			new mem_ready_event_t(
				parent.second, event->data, clock.get() + 1));
		events->push_back(
			new mem_retire_event_t(
				parent.second, event->data, clock.get() + read_latency));
	}
}

void main_mem_t::process(mem_write_event_t *event){
	TIME_VIOLATION_CHECK

	auto bank = get_bank(event->data.addr);
	if(promote_pending(event, [&, bank](){
		if(banks[bank].writers < write_ports_per_bank &&
			banks[bank].total() < ports_per_bank) return false;
		return true;
	}) != nullptr) {
		banks[bank].writerq++;
		if(banks[bank].writerq <= store_buf_size) {
			for(auto parent : parents.raw<ram_signal_handler_t *>()) {
				events->push_back(
					new mem_ready_event_t(
						parent.second, event->data, clock.get() + 1));
			}
		}
		return;
	}

	writes.inc();

	// Increment writers
	banks[bank].writers++;
	if(banks[bank].writerq > 0) banks[bank].writerq--;
	auto pending_event = new pending_event_t(
		this, nullptr, clock.get() + write_latency);
	pending_event->add_fini([&, bank](){ banks[bank].writers--; });
	register_pending(pending_event);
	events->push_back(pending_event);

	for(auto parent : parents.raw<ram_signal_handler_t *>()) {
		events->push_back(
			new mem_ready_event_t(
				parent.second, event->data, clock.get() + 1));
		events->push_back(
			new mem_retire_event_t(
				parent.second, event->data, clock.get() + write_latency));
	}
}