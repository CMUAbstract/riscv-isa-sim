#ifndef MEM_H
#define MEM_H

#include <fesvr/memif.h>

#include "component.h"

struct mem_read_event_t;
struct mem_write_event_t;
struct mem_insert_event_t;
struct mem_ready_event_t;
struct mem_stall_event_t;
struct ready_event_t;
struct stall_event_t;
class mem_t: public component_t {
public:
	using component_t::component_t;
	virtual void process(mem_read_event_t *event) = 0;
	virtual void process(mem_write_event_t *event) = 0;
	virtual void process(mem_insert_event_t *event) = 0;
	virtual void process(ready_event_t *event) = 0;
	virtual void process(stall_event_t *event) = 0;
};

template<typename T> 
mem_t* create_mem(std::string name, io::json config, event_list_t *events) {
	return new T(name, config, events);
}

#endif