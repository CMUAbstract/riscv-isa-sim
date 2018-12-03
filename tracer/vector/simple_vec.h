#ifndef SIMPLE_VEC_H
#define SIMPLE_VEC_H

#include "vcu.h"

class simple_vec_t: public vcu_t {
public:
	simple_vec_t(std::string _name, io::json _config, event_heap_t *_events);

	void process(vector_exec_event_t *event);
	void process(pe_exec_event_t *event);
	void process(pe_ready_event_t *event);
	void process(pending_event_t *event);
};

#endif