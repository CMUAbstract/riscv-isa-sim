#ifndef MAIN_MEM_H
#define MAIN_MEM_H

#include <vector>
#include <tuple>

#include <stat/stat.h>

#include "ram.h"

class main_mem_t: public ram_t {
public:
	main_mem_t(std::string _name, io::json _config, event_heap_t *_events);	
	virtual ~main_mem_t() {}
	void process(mem_read_event_t *event);
	void process(mem_write_event_t *event);
	void process(mem_insert_event_t *event) {}
};

#endif