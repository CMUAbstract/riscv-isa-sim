#ifndef MEM_EVENT_H
#define MEM_EVENT_H

#include <sstream>

#include "event.h"
#include "core_handler.h"
#include "ram_handler.h"

struct reg_read_event_t: public event_t<core_handler_t, reg_t> {
	using event_t<core_handler_t, reg_t>::event_t;
	std::string to_string() {
		std::ostringstream os;
		os << "reg_read_event (" << cycle << "," << data << ")"; 
		return os.str();
	}
	std::string get_name() { return "reg_read_event"; }
	HANDLER;
};

struct reg_write_event_t: public event_t<core_handler_t, reg_t> {
	using event_t<core_handler_t, reg_t>::event_t;
	std::string to_string() {
		std::ostringstream os;
		os << "reg_write_event (" << cycle << "," << data << ")"; 
		return os.str();
	}
	std::string get_name() { return "reg_write_event"; }
	HANDLER;
};

struct mem_event_t: public event_t<ram_handler_t, addr_t> {
	using event_t<ram_handler_t, addr_t>::event_t;
	std::string to_string() {
		std::ostringstream os;
		os << " (" << this->cycle << ", 0x" << std::hex << this->data << ")"; 
		return os.str();
	}
};

struct mem_read_event_t: public mem_event_t {
	using mem_event_t::mem_event_t;
	std::string to_string() {
		std::string o = "mem_read_event";
		return o + mem_event_t::to_string();
	}
	std::string get_name() { return "mem_read_event"; }
	HANDLER;
};

struct mem_write_event_t: public mem_event_t {
	using mem_event_t::mem_event_t;
	std::string to_string() {
		std::string o = "mem_write_event";
		return o + mem_event_t::to_string();
	}
	std::string get_name() { return "mem_write_event"; }
	HANDLER;
};

struct mem_insert_event_t: public mem_event_t {
	using mem_event_t::mem_event_t;
	std::string to_string() {
		std::string o = "mem_insert_event";
		return o + mem_event_t::to_string();
	}
	std::string get_name() { return "mem_insert_event"; }
	HANDLER;
};

struct mem_signal_event_t: public event_t<ram_signal_handler_t, addr_t> {
	using event_t<ram_signal_handler_t, addr_t>::event_t;
	std::string to_string() {
		std::ostringstream os;
		os << " (" << this->cycle << ", 0x" << std::hex << this->data << ")"; 
		return os.str();
	}
};

struct mem_ready_event_t: public mem_signal_event_t{
	using mem_signal_event_t::mem_signal_event_t;
	std::string to_string() {
		std::ostringstream os;
		os << "mem_ready_event (" << cycle << ", 0x" << std::hex << this->data << ")"; 
		return os.str();
	}
	std::string get_name() { return "mem_ready_event"; }
	HANDLER;
};

struct mem_match_event_t: public mem_signal_event_t{
	using mem_signal_event_t::mem_signal_event_t;
	std::string to_string() {
		std::ostringstream os;
		os << "mem_match_event (" << cycle << ", 0x" << std::hex << this->data << ")"; 
		return os.str();
	}
	std::string get_name() { return "mem_ready_event"; }
	HANDLER;
};

#endif