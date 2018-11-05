#ifndef MEM_H
#define MEM_H

#include <fesvr/memif.h>

#include "component.h"
#include "signal.h"

struct mem_read_event_t;
struct mem_write_event_t;
struct mem_insert_event_t;
class ram_t: public component_t, public signal_handler_t {
public:
	using component_t::component_t;
	virtual ~ram_t() {}
	virtual void process(mem_read_event_t *event) = 0;
	virtual void process(mem_write_event_t *event)= 0;
	virtual void process(mem_insert_event_t *event) = 0;
};

#endif