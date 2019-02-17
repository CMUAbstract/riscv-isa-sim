#ifndef VEC1D_FLOW_VEC_H
#define VEC1D_FLOW_VEC_H

#include <vector>

#include "vcu.h"

class vec1dflow_t: public vcu_t {
public:
	vec1dflow_t(std::string _name, io::json _config, event_heap_t *_events);
	io::json to_json() const;
	void reset(reset_level_t level);
	void process(vector_exec_event_t *event);
	void process(vector_start_event_t *event);
	void process(pe_exec_event_t *event);
	void process(pe_ready_event_t *event);
protected:
	uint32_t window_size = 1;
	bool src_forwarding = false;

	bool start = false;
	uint16_t idx = 0;
	uint16_t window_start = 0;
	uint16_t active_insn_offset = 0;
	uint16_t active_window_size = 0;

	std::vector<bool> reg_map; // true buffer, false vrf 
	std::vector<bool> kill_map; // true if killed
	std::vector<bool> src_map; // true if source forwarded
	std::vector<uint16_t> progress_map;
};

#endif