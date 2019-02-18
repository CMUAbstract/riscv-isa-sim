#ifndef VEC1D_MULTI_H
#define VEC1D_MULTI_H

#include <set>
#include <vector>

#include "vcu.h"

class vec1dmulti_t: public vcu_t {
public:
	vec1dmulti_t(std::string _name, io::json _config, event_heap_t *_events);
	io::json to_json() const;
	void reset(reset_level_t level);
	void process(vector_exec_event_t *event);
	void process(pe_exec_event_t *event);
	void process(pe_ready_event_t *event);
protected:
	uint32_t window_size = 1;
	uint32_t rf_ports = 1;

	uint16_t idx = 0;
	bool start = false;
	uint16_t active_window_size = 0;
	uint16_t active_insn_offset = 0;
	uint16_t active_reg_reads = 0;
	uint16_t active_reg_writes = 0;
	uint16_t progress = 0;

	std::set<uint8_t> write_set;
	std::set<uint8_t> read_set;

};

#endif