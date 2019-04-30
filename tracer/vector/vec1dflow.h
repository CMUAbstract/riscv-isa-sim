#ifndef VEC1D_FLOW_VEC_H
#define VEC1D_FLOW_VEC_H

#include <vector>

#include "vcu.h"

class vec1dflow_t: public vcu_t {
public:
	vec1dflow_t(std::string _name, io::json _config, value_heap_t *_values);
	io::json to_json() const;
	void reset(reset_level_t level);
	void process(vec_issue_value_tvalue);
	void process(vec_start_value_tvalue);
	void process(pe_exec_value_tvalue);
	void process(pe_ready_value_tvalue);
protected:
	uint32_t window_size = 1;
	bool m2m = false;
	bool forwarding = true;
	bool src_forwarding = false;

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