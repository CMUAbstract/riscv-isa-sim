#ifndef CORE_EVENT_H
#define CORE_EVENT_H

#include <sstream>

#include <common/decode.h>

#include "smartptr.h"
#include "event.h"
#include "core.h"

class working_set_t;
struct timed_insn_t {
	timed_insn_t();
	timed_insn_t(shared_ptr_t<working_set_t> _ws, 
		insn_bits_t _opc, insn_t _insn);
	timed_insn_t(timed_insn_t *_insn);
	~timed_insn_t() {}
	shared_ptr_t<working_set_t> ws;
	insn_bits_t opc;
	insn_t insn;
};

struct insn_event_t: public event_t<core_handler_t, timed_insn_t> {
	insn_event_t(core_handler_t *_handler, timed_insn_t *_insn) 
		: event_t<core_handler_t, timed_insn_t>(_handler, _insn) {}
	std::string to_string();
};

struct insn_fetch_event_t: public insn_event_t {
	using insn_event_t::insn_event_t;
	std::string to_string() {
		std::string o = "insn_fetch_event";
		return o + insn_event_t::to_string();
	}
	HANDLER;
};

struct insn_decode_event_t: public insn_event_t {
	using insn_event_t::insn_event_t;
	std::string to_string() {
		std::string o = "insn_decode_event";
		return o + insn_event_t::to_string();
	}
	HANDLER;
};

struct insn_exec_event_t: public insn_event_t {
	using insn_event_t::insn_event_t;
	std::string to_string() {
		std::string o = "insn_exec_event";
		return o + insn_event_t::to_string();
	}
	HANDLER;
};

struct insn_retire_event_t: public insn_event_t {
	using insn_event_t::insn_event_t;
	std::string to_string() {
		std::string o = "insn_retire_event";
		return o + insn_event_t::to_string();
	}
	HANDLER;
};

#endif