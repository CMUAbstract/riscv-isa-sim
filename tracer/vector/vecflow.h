#ifndef VEC_FLOW_H
#define VEC_FLOW_H

#include <vector>

#include "vcu.h"

class vecflow_t: public vcu_t {
public:
	vecflow_t(std::string _name, io::json _config, event_heap_t *_events);
	void process(vector_exec_event_t *event);
	void process(pe_exec_event_t *event);
	void process(pe_ready_event_t *event);
protected:
	uint16_t window_size;
	uint32_t idx = 0;
	std::vector<uint32_t> indices;
	std::vector<uint32_t> active_lanes;
	uint32_t active_window_size = 0;
};

#endif