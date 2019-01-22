#ifndef VEC_FLOW_H
#define VEC_FLOW_H

#include <vector>
#include <set>

#include "vcu.h"

class vecflow_t: public vcu_t {
public:
	vecflow_t(std::string _name, io::json _config, event_heap_t *_events);
	void process(vector_exec_event_t *event);
	void process(pe_exec_event_t *event);
	void process(pe_ready_event_t *event);
protected:
	struct pipeline_reg_info_t {
		uint32_t reg;
		uint32_t vreg;
		uint32_t idx;
	};
	struct pipeline_reg_read_event_t: event_t<vecflow_t, pipeline_reg_info_t> {
		using event_t<vecflow_t, pipeline_reg_info_t>::event_t;
		std::string to_string() {
			std::ostringstream os;
			os << "pipeline_reg_read_event (" << cycle << ","; 
			os << data.reg << ", " << data.vreg << ", " << data.idx << ")"; 
			return os.str();	
		}
		std::string get_name() { return "pipeline_reg_read_event"; }
		HANDLER;
	};
	struct pipeline_reg_write_event_t: event_t<vecflow_t, pipeline_reg_info_t> {
		using event_t<vecflow_t, pipeline_reg_info_t>::event_t;
		std::string to_string() {
			std::ostringstream os;
			os << "pipeline_reg_write_event (" << cycle << ","; 
			os << data.reg << ", " << data.vreg << ", " << data.idx << ")"; 
			return os.str();
		}
		std::string get_name() { return "pipeline_reg_write_event"; }
		HANDLER;
	};
	void process(pipeline_reg_read_event_t *event);
	void process(pipeline_reg_write_event_t *event);
protected:
	std::set<uint32_t> get_reg_set(uint32_t idx);
protected:
	uint16_t window_size;
	uint32_t idx = 0;
	std::vector<uint32_t> indices;
	std::vector<uint32_t> active_lanes;
	std::vector<std::set<uint32_t>> outputs;
	uint32_t active_window_size = 0;
};

#endif