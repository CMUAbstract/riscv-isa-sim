#ifndef VEC1D_FLOW_VEC_H
#define VEC1D_FLOW_VEC_H

#include <vector>

#include "vcu.h"

class vec1dflow_t: public vcu_t {
public:
	vec1dflow_t(std::string _name, io::json _config, event_heap_t *_events);
	io::json to_json() const;
	void reset(reset_level_t level);
	void process(vec_issue_event_t *event);
	void process(vec_start_event_t *event);
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

	enum forward_state_t {REG, FORWARD, BOTH};
	struct reg_info_t {
		uint16_t op1 = 0;
		uint16_t op2 = 0;
		uint16_t result = 0;
		bool op1f = false; // True if previously forwarded
		bool op2f = false; // True if previously forwarded
		forward_state_t resultf = REG;
	};

	std::vector<reg_info_t> reg_info; 
	std::vector<uint16_t> progress_map;
};

#endif