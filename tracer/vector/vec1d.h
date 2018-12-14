#ifndef SIMPLE_VEC_H
#define SIMPLE_VEC_H

#include <deque>

#include "vcu.h"

class vec1d_t: public vcu_t {
public:
	vec1d_t(std::string _name, io::json _config, event_heap_t *_events);

	void process(vector_exec_event_t *event);
	void process(pe_exec_event_t *event);
	void process(pe_ready_event_t *event);
	void process(vector_reg_read_event_t *event);
	void process(vector_reg_write_event_t *event);
	void process(mem_ready_event_t *event);
	void process(mem_retire_event_t *event);
	void process(mem_match_event_t *event);
protected:
	// Characteristics
	uint32_t lanes;
	// Data
	std::deque<uint32_t> state; // Keep index for each instruction
	uint32_t retired_idx = 0;
};

#endif