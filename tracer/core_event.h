#ifndef CORE_EVENT_H
#define CORE_EVENT_H

#include <sstream>

#include <common/decode.h>

#include "smartptr.h"
#include "event.h"
#include "core.h"

struct timed_insn_t;
struct insn_event_t: public event_t<core_handler_t, shared_ptr_t<timed_insn_t>> {
	using event_t<core_handler_t, shared_ptr_t<timed_insn_t>>::event_t;
	std::string to_string();
};

struct insn_fetch_event_t: public insn_event_t {
	using insn_event_t::insn_event_t;
	std::string to_string() {
		std::string o = "insn_fetch_event";
		return o + insn_event_t::to_string();
	}
	std::string get_name() { return "insn_fetch_event"; }
	HANDLER;
};

struct insn_decode_event_t: public insn_event_t {
	using insn_event_t::insn_event_t;
	std::string to_string() {
		std::string o = "insn_decode_event";
		return o + insn_event_t::to_string();
	}
	std::string get_name() { return "insn_decode_event"; }
	HANDLER;
};

struct insn_exec_event_t: public insn_event_t {
	using insn_event_t::insn_event_t;
	std::string to_string() {
		std::string o = "insn_exec_event";
		return o + insn_event_t::to_string();
	}
	std::string get_name() { return "insn_exec_event"; }
	HANDLER;
};

struct insn_retire_event_t: public insn_event_t {
	using insn_event_t::insn_event_t;
	std::string to_string() {
		std::string o = "insn_retire_event";
		return o + insn_event_t::to_string();
	}
	std::string get_name() { return "insn_retire_event"; }
	HANDLER;
};

#endif