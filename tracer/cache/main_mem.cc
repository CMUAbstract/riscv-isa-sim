#include "main_mem.h"

#include "mem_event.h"
#include "pending_event.h"

main_mem_t::main_mem_t(std::string _name, io::json _config, event_heap_t *_events)
	: ram_t(_name, _config, _events) {
	JSON_CHECK(int, config["read_latency"], read_latency, 1);
	JSON_CHECK(int, config["write_latency"], write_latency, 1);
	JSON_CHECK(int, config["banks"], bank_count, 1);
	JSON_CHECK(int, config["ports"], ports, 0);
	JSON_CHECK(int, config["read_ports"], read_ports, 0);
	JSON_CHECK(int, config["write_ports"], write_ports, 0);
	
	// Calculate number of ports and ports/bank
	assert_msg(ports > 0 || (read_ports > 0 && write_ports > 0),
		"either ports > 0 or (read_ports > = and write_ports > 0)"
		);
	assert_msg(ports % bank_count == 0 
		|| (read_ports % bank_count == 0 && write_ports % bank_count == 0),
		"port - bank mismatch");
	banks.resize(bank_count, std::make_tuple(0, 0));
	if(read_ports == 0) total_ports = true;
	if(ports > 0) total_ports = true;
	ports_per_bank = ports / bank_count;
	read_ports_per_bank = read_ports / bank_count;
	write_ports_per_bank = write_ports / bank_count;	
	bank_mask = bank_count - 1;
}

void main_mem_t::process(mem_read_event_t *event){
	TIME_VIOLATION_CHECK
	
	auto bank = get_bank(event->data);
	if(promote_pending(event, [&, bank](){
		if(total_ports && std::get<0>(banks[bank]) < ports_per_bank) return false;
		else if(std::get<0>(banks[bank]) < read_ports_per_bank) return false; 
		return true;
	}) != nullptr) return;

	reads.inc();

	// Increment readers
	std::get<0>(banks[bank])++;
	auto pending_event = new pending_event_t(
		this, nullptr, clock.get() + read_latency);
	pending_event->add_fini([&](){ std::get<0>(banks[bank])--; });
	register_pending(pending_event);
	events->push_back(pending_event);

	for(auto parent : parents.raw<ram_t *>()) {
		events->push_back(
			new mem_insert_event_t(
				parent.second, event->data, clock.get() + read_latency));
	}
	for(auto parent : parents.raw<ram_signal_handler_t *>()) {
		events->push_back(
			new mem_ready_event_t(
				parent.second, event->data, clock.get() + read_latency));
		events->push_back(
			new mem_retire_event_t(
				parent.second, event->data, clock.get() + read_latency));
	}
}

void main_mem_t::process(mem_write_event_t *event){
	TIME_VIOLATION_CHECK

	auto bank = get_bank(event->data);
	if(promote_pending(event, [&, bank](){
		if(total_ports && std::get<0>(banks[bank]) < ports_per_bank) return false;
		else if(std::get<1>(banks[bank]) < write_ports_per_bank) return false; 
		return true;
	}) != nullptr) return;

	writes.inc();

	// Increment writers
	if(total_ports) std::get<0>(banks[bank])++;
	else std::get<1>(banks[bank])++;
	auto pending_event = new pending_event_t(
		this, nullptr, clock.get() + write_latency);
	pending_event->add_fini([&](){ 
		if(total_ports) std::get<0>(banks[bank])--;
		else std::get<1>(banks[bank])--;
	});
	register_pending(pending_event);
	events->push_back(pending_event);

	for(auto parent : parents.raw<ram_t *>()) {
		events->push_back(
			new mem_insert_event_t(
				parent.second, event->data, clock.get() + write_latency));
	}
	for(auto parent : parents.raw<ram_signal_handler_t *>()) {
		events->push_back(
			new mem_ready_event_t(
				parent.second, event->data, clock.get() + 1));
		events->push_back(
			new mem_retire_event_t(
				parent.second, event->data, clock.get() + write_latency));
	}
}

addr_t main_mem_t::get_bank(addr_t addr) {
	return addr & bank_mask;
}
