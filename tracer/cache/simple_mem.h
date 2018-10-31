#ifndef SIMPLE_MEM_H

#include "mem.h"

class simple_mem_t: public mem_t {
public:
	using mem_t::mem_t;
	void process(mem_read_event_t *event);
	void process(mem_write_event_t *event);
	void process(mem_insert_event_t *event);
	void process(mem_ready_event_t *event);
	void process(mem_stall_event_t *event);
};

#endif