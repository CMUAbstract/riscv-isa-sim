#ifndef CORE_H
#define CORE_H

#include <common/decode.h>

#include "event.h"
#include "mem.h"

class working_set_t;
template <typename T>
struct insn_event_t : public event_t<T> {
	insn_event_t(T *_handler, working_set_t *_ws, insn_bits_t _opc, 
		insn_t _insn) : event_t<T>(_handler), ws(_ws), opc(_opc), insn(_insn) {}
	insn_event_t(working_set_t *_ws, insn_bits_t _opc, insn_t _insn) 
		: ws(_ws), opc(_opc), insn(_insn) {}
	HANDLER;
	working_set_t *ws;
	insn_bits_t opc;
	insn_t insn;	
};

class core_t: public component_t {
public:
	using component_t::component_t;
	void process(event_t<core_t> *event) {}
	virtual void process(insn_event_t<core_t> *event) = 0;
	virtual void process(reg_read_event_t<core_t> *event) = 0;
	virtual void process(reg_write_event_t<core_t> *event) = 0;
};

#endif