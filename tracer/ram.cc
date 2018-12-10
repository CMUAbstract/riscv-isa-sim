#include "ram.h"

ram_t::ram_t(std::string _name, io::json _config, event_heap_t *_events) 
	: component_t(_name, _config, _events), reads("reads"), writes("writes") {
	JSON_CHECK(int, config["read_latency"], read_latency, 1);
	JSON_CHECK(int, config["write_latency"], write_latency, 1);
	JSON_CHECK(int, config["banks"], bank_count, 1);
	JSON_CHECK(int, config["ports"], ports, 1);
	JSON_CHECK(int, config["read_ports"], read_ports, 0);
	JSON_CHECK(int, config["write_ports"], write_ports, 0);
	JSON_CHECK(int, config["load_buf_size"], load_buf_size, 0);
	JSON_CHECK(int, config["store_buf_size"], store_buf_size, 0);

	// Calculate number of ports and ports/bank
	assert_msg(ports > 0 || (read_ports > 0 && write_ports > 0),
		"either ports > 0 or (read_ports > = and write_ports > 0)");
	assert_msg(ports % bank_count == 0 || 
		(read_ports % bank_count == 0 && write_ports % bank_count == 0),
		"port - bank mismatch");
	banks.resize(bank_count);
	ports_per_bank = ports / bank_count;
	read_ports_per_bank = read_ports / bank_count;
	write_ports_per_bank = write_ports / bank_count;	
	if(read_ports > 0) {
		ports_per_bank = read_ports_per_bank + write_ports_per_bank;
	} else {
		read_ports_per_bank = ports_per_bank;
		write_ports_per_bank = ports_per_bank;
	}
	bank_mask = bank_count - 1;

	pending_handler_t::set_ref(events);
	reads.reset();
	writes.reset();
}

void ram_t::reset() {
	component_t::reset();
	clear_pending();	
}

io::json ram_t::to_json() const {
	return io::json::merge_objects(component_t::to_json(), reads, writes);
}

addr_t ram_t::get_bank(addr_t addr) {
	return addr & bank_mask;
}