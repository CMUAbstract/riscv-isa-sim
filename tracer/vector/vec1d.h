#ifndef SIMPLE_VEC_H
#define SIMPLE_VEC_H

#include "vcu.h"

class vec1d_t: public vcu_t {
public:
	vec1d_t(std::string _name, io::json _config, event_heap_t *_events)
		: vcu_t(_name, _config, _events) {}
	void process(vector_exec_event_t *event);
	void process(pe_exec_event_t *event);
	void process(pe_ready_event_t *event);
protected:
	// Data
	uint32_t idx = 0;
	uint32_t active_lanes = 0;
};

#endif