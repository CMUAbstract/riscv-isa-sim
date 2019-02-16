#ifndef VEC1D_MULTI_H
#define VEC1D_MULTI_H

#include <vector>

#include "vcu.h"

#define VREG_COUNT 0x10

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
	bool src_forwarding = false;
};

#endif