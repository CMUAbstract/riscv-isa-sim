#ifndef VECTOR_EVENT_H
#define VECTOR_EVENT_H

#include "event.h"
#include "vector_handler.h"
#include "core_event.h"

struct vec_ready_event_t: public insn_event_t<vec_signal_handler_t> {
	using insn_event_t<vec_signal_handler_t>::insn_event_t;
	std::string to_string() {
		std::string o = "vec_ready_event";
		return o + insn_event_t<vec_signal_handler_t>::to_string();
	}
	std::string get_name() { return "vec_ready_event"; }
	HANDLER;
};

struct vec_retire_event_t: public insn_event_t<vec_signal_handler_t> {
	using insn_event_t<vec_signal_handler_t>::insn_event_t;
	std::string to_string() {
		std::string o = "vec_retire_event_t";
		return o + insn_event_t<vec_signal_handler_t>::to_string();
	}
	std::string get_name() { return "vec_retire_event_t"; }
	HANDLER;

};

struct vec_issue_event_t: public insn_event_t<vec_handler_t> {
	using insn_event_t<vec_handler_t>::insn_event_t;
	std::string to_string() {
		std::string o = "vec_issue_event";
		return o + insn_event_t<vec_handler_t>::to_string();
	}
	std::string get_name() { return "vec_issue_event"; }
	HANDLER;
};

struct vec_start_event_t: public event_t<vec_handler_t, bool> {
	using event_t<vec_handler_t, bool>::event_t;
	std::string to_string() { return "vec_start_event"; }
	std::string get_name() { return to_string(); }
	HANDLER;
};

struct pe_exec_event_t: public insn_event_t<vec_handler_t> {
	using insn_event_t<vec_handler_t>::insn_event_t;
	std::string to_string() {
		std::string o = "pe_exec_event";
		return o + insn_event_t<vec_handler_t>::to_string();
	}
	std::string get_name() { return "pe_exec_event"; }
	HANDLER;
};

struct pe_ready_event_t: public insn_event_t<vec_handler_t> {
	using insn_event_t<vec_handler_t>::insn_event_t;
	std::string to_string() {
		std::string o = "pe_ready_event";
		return o + insn_event_t<vec_handler_t>::to_string();
	}
	std::string get_name() { return "pe_ready_event"; }
	HANDLER;
};

struct vec_reg_info_t {
	reg_t reg;
	uint32_t idx;
};

struct vec_reg_read_event_t: public event_t<vec_handler_t, vec_reg_info_t> {
	using event_t<vec_handler_t, vec_reg_info_t>::event_t;
	std::string to_string() {
		std::ostringstream os;
		os << "vec_reg_read_event (" << cycle << ","; 
		os << data.reg << ", " << data.idx << ")"; 
		return os.str();
	}
	std::string get_name() { return "vec_reg_read_event"; }
	HANDLER;
};

struct vec_reg_write_event_t: public event_t<vec_handler_t, vec_reg_info_t> {
	using event_t<vec_handler_t, vec_reg_info_t>::event_t;
	std::string to_string() {
		std::ostringstream os;
		os << "vec_reg_write_event (" << cycle << ","; 
		os << data.reg << ", " << data.idx << ")"; 
		return os.str();
	}
	std::string get_name() { return "vec_reg_write_event"; }
	HANDLER;
};

#endif