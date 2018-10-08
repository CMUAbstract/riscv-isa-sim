#ifndef CORE_H
#define CORE_H

#include <common/decode.h>

#include "event.h"

class working_set_t;
struct insn_event_t : public event_t {
	insn_event_t(working_set_t *_ws, insn_bits_t _opc, insn_t _insn) 
		: ws(_ws), opc(_opc), insn(_insn) {}
	working_set_t *ws;
	insn_bits_t opc;
	insn_t insn;	
};

class core_t: public component_t {
public:
	using component_t::component_t;
	void process(event_t event) {}
	virtual void process(insn_event_t event) = 0;
};

#endif