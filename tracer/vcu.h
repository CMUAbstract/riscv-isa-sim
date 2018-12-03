#ifndef VCU_H
#define VCU_H

#include "component.h"
#include "pending_handler.h"
#include "vector_handler.h"

class vcu_t: public component_t<vcu_t, vector_handler_t, pending_handler_t> {
public:
	vcu_t(std::string _name, io::json _config, event_heap_t *_events);
	~vcu_t() {}
	bool check_vec(insn_bits_t opc);
	virtual void reset() {}
	virtual io::json to_json() const;
};

#endif