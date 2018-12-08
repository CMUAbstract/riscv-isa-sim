#ifndef MAIN_MEM_H
#define MAIN_MEM_H

#include "ram.h"

class main_mem_t: public ram_t {
public:
	main_mem_t(std::string _name, io::json _config, event_heap_t *_events);	
	virtual ~main_mem_t() {}
	void process(mem_read_event_t *event);
	void process(mem_write_event_t *event);
	void process(pending_event_t *event);
	void process(mem_insert_event_t *event) {}
	void process(mem_ready_event_t *event) {}
	void process(mem_match_event_t *event) {}
protected:
	uint32_t read_latency = 1;
	uint32_t write_latency = 1;
	uint32_t ports = 1;
};

#endif