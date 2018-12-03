#ifndef VCU_H
#define VCU_H

#include "component.h"
#include "pending_handler.h"
#include "vector_handler.h"
#include "core.h"

class vcu_t: public component_t<vcu_t, vector_handler_t, pending_handler_t> {
public:
	vcu_t(std::string _name, io::json _config, event_heap_t *_events);
	~vcu_t() {}
	bool check_vec(insn_bits_t opc);
	void check_and_set_vl(hstd::shared_ptr<timed_insn_t> insn);
	virtual void reset() {}
	virtual io::json to_json() const;
protected:
	reg_t vl;
};

#endif