#ifndef MEM_H
#define MEM_H

#include <fesvr/memif.h>

#include "event.h"

class mem_read_event_t;
class mem_write_event_t;
class mem_miss_event_t;
class mem_invalidate_event_t;
class mem_t: public component_t {
public:
	using component_t::component_t;
	virtual void process(mem_read_event_t *event) = 0;
	virtual void process(mem_write_event_t *event) = 0;
	virtual void process(mem_miss_event_t *event) = 0;
	virtual void process(mem_invalidate_event_t *event) = 0;
};

template<typename T> mem_t* create_mem(io::json config, event_list_t *events) {
	return new T(config, events);
}

#endif