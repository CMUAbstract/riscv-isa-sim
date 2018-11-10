#ifndef MAIN_MEM_H
#define MAIN_MEM_H

#include "ram.h"

class main_ram_t: public ram_t {
public:
	main_ram_t(std::string _name, io::json _config, event_list_t *_events);	
	virtual ~main_ram_t() {}
	void process(mem_read_event_t *event);
	void process(mem_write_event_t *event);
	void process(mem_insert_event_t *event) {}
	void process(ready_event_t *event) {}
	void process(stall_event_t *event) {}
protected:
	uint32_t read_latency;
	uint32_t write_latency;
};

#endif