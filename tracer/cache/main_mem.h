#ifndef MAIN_MEM_H
#define MAIN_MEM_H

#include <vector>
#include <tuple>

#include "ram.h"

class main_mem_t: public ram_t {
public:
	main_mem_t(std::string _name, io::json _config, event_heap_t *_events);	
	virtual ~main_mem_t() {}
	void process(mem_read_event_t *event);
	void process(mem_write_event_t *event);
	void process(pending_event_t *event);
	void process(mem_insert_event_t *event) {}
protected:
	addr_t get_bank(addr_t addr);
protected:
	uint32_t read_latency;
	uint32_t write_latency;
	uint32_t ports;
	uint32_t bank_count;
	uint32_t read_ports;
	uint32_t write_ports;

	bool total_ports = false;
	uint32_t ports_per_bank;
	uint32_t read_ports_per_bank;
	uint32_t write_ports_per_bank;
	uint32_t bank_mask;

	std::vector<std::tuple<uint32_t, uint32_t>> banks;
};

#endif