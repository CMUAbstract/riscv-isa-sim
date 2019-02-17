#ifndef VCU_H
#define VCU_H

#include <string>
#include <vector>

#include "component.h"
#include "pending_handler.h"
#include "vector_handler.h"
#include "ram.h"
#include "core.h"

class vcu_t: public component_t<vcu_t, vector_handler_t, 
	pending_handler_t, ram_signal_handler_t> {
public:
	vcu_t(std::string _name, io::json _config, event_heap_t *_events);
	~vcu_t() {}
	void set_core(core_t *_core) { core = _core; }
	bool check_vec(insn_bits_t opc);
	bool check_empty() { return empty; }
	bool check_split(insn_bits_t opc);
	void check_and_set_vl(hstd::shared_ptr<timed_insn_t> insn);
	virtual void reset(reset_level_t level);
	virtual io::json to_json() const;
	void process(vector_start_event_t *event);
	void process(vector_reg_read_event_t *event);
	void process(vector_reg_write_event_t *event);
	void process(mem_ready_event_t *event);
	void process(mem_retire_event_t *event);
protected:
	void set_core_stage(std::string stage, bool val) { core->stages[stage] = val; }
	bool check_killed(uint8_t reg) { return (reg & 0x10) > 0; }
	uint8_t strip_killed(uint8_t reg) { return reg & 0xf; }
	reg_t vl;
	core_t *core;
	uint16_t reg_count;
	uint16_t lanes;
	uint32_t outstanding = 0;
	bool empty = true;
};

#endif