#ifndef VECTOR_EVENT_H
#define VECTOR_EVENT_H

#include "event.h"
#include "vector_handler.h"
#include "core_event.h"

struct vector_ready_event_t: public insn_event_t<vector_signal_handler_t> {
	using insn_event_t<vector_signal_handler_t>::insn_event_t;
	std::string to_string() {
		std::string o = "vector_ready_event";
		return o + insn_event_t<vector_signal_handler_t>::to_string();
	}
	std::string get_name() { return "vector_ready_event"; }
	HANDLER;

};

struct vector_exec_event_t: public insn_event_t<vector_handler_t> {
	using insn_event_t<vector_handler_t>::insn_event_t;
	std::string to_string() {
		std::string o = "vector_exec_event";
		return o + insn_event_t<vector_handler_t>::to_string();
	}
	std::string get_name() { return "vector_exec_event"; }
	HANDLER;
};

struct pe_exec_event_t: public insn_event_t<vector_handler_t> {
	using insn_event_t<vector_handler_t>::insn_event_t;
	std::string to_string() {
		std::string o = "pe_exec_event";
		return o + insn_event_t<vector_handler_t>::to_string();
	}
	std::string get_name() { return "pe_exec_event"; }
	HANDLER;

};

struct pe_ready_event_t: public insn_event_t<vector_handler_t> {
	using insn_event_t<vector_handler_t>::insn_event_t;
	std::string to_string() {
		std::string o = "pe_ready_event";
		return o + insn_event_t<vector_handler_t>::to_string();
	}
	std::string get_name() { return "pe_ready_event"; }
	HANDLER;

};

#endif