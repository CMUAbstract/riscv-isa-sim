#ifndef INSN_EVENT_H
#define INSN_EVENT_H

#include <common/decode.h>

#include "smartptr.h"
#include "event.h"
#include "core.h"

class working_set_t;
struct timed_insn_t {
	timed_insn_t(shared_ptr_t<working_set_t> _ws, 
		insn_bits_t _opc, insn_t _insn);
	timed_insn_t(timed_insn_t *_insn);
	~timed_insn_t() {}
	shared_ptr_t<working_set_t> ws;
	insn_bits_t opc;
	insn_t insn;
};

struct insn_event_t: public event_t<core_t> {
	insn_event_t(core_t *_handler, shared_ptr_t<working_set_t> _ws, 
		insn_bits_t _opc, insn_t _insn)
		: event_t<core_t>(_handler), insn(_ws, _opc, _insn) {}
	insn_event_t(shared_ptr_t<working_set_t> _ws, 
		insn_bits_t _opc, insn_t _insn) 
		: insn(_ws, _opc, _insn) {}
	insn_event_t(core_t *_handler, timed_insn_t *_insn) 
		: event_t<core_t>(_handler), insn(_insn) {}
	insn_event_t(timed_insn_t *_insn) : insn(_insn) {}
	timed_insn_t insn;
};

struct insn_fetch_event_t: public insn_event_t {
	using insn_event_t::insn_event_t;
	HANDLER;
};

struct insn_decode_event_t: public insn_event_t {
	using insn_event_t::insn_event_t;
	HANDLER;
};

#endif