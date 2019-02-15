#ifndef VEC1D_FLOW_VEC_H
#define VEC1D_FLOW_VEC_H

#include <vector>

#include "vcu.h"

#define VREG_COUNT 0x10

class vec1dflow_t: public vcu_t {
public:
	vec1dflow_t(std::string _name, io::json _config, event_heap_t *_events);
	io::json to_json() const;
	void reset(reset_level_t level);
	void process(vector_exec_event_t *event);
	void process(pe_exec_event_t *event);
	void process(pe_ready_event_t *event);
protected:
	uint32_t window_size = 1;
	bool src_forwarding = false;
	// Data
	uint16_t idx = 0;
	uint16_t window_start = 0;
	uint16_t active_insn_offset = 0;
	uint16_t active_window_size = 0;
	bool active_running = false;
	std::vector<uint16_t> progress_map;
	std::vector<bool> reg_map; // true buffer, false vrf 
	std::vector<bool> kill_map; // true if killed
};

#endif