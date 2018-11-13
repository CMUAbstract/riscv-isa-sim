#ifndef MEM_EVENT_H
#define MEM_EVENT_H

#include <sstream>

#include "event.h"
#include "core.h"
#include "ram.h"

struct reg_read_event_t: public event_t<core_handler_t, reg_t> {
	using event_t<core_handler_t, reg_t>::event_t;
	std::string to_string() {
		std::ostringstream os;
		os << "reg_read_event (" << cycle << "," << data << ")"; 
		return os.str();
	}
	HANDLER;
};

struct reg_write_event_t: public event_t<core_handler_t, reg_t> {
	using event_t<core_handler_t, reg_t>::event_t;
	std::string to_string() {
		std::ostringstream os;
		os << "reg_write_event (" << cycle << "," << data << ")"; 
		return os.str();
	}
	HANDLER;
};

struct mem_event_t: public event_t<ram_handler_t, addr_t> {
	using event_t<ram_handler_t, addr_t>::event_t;
	std::string to_string() {
		std::ostringstream os;
		os << " (" << cycle << ", 0x" << std::hex << data << ")"; 
		return os.str();
	}
};

struct mem_read_event_t: public mem_event_t {
	using mem_event_t::mem_event_t;
	std::string to_string() {
		std::string o = "mem_read_event";
		return o + mem_event_t::to_string();
	}
	HANDLER;
};

struct mem_write_event_t: public mem_event_t {
	using mem_event_t::mem_event_t;
	std::string to_string() {
		std::string o = "mem_write_event";
		return o + mem_event_t::to_string();
	}
	HANDLER;
};

struct mem_insert_event_t: public mem_event_t {
	using mem_event_t::mem_event_t;
	std::string to_string() {
		std::string o = "mem_insert_event";
		return o + mem_event_t::to_string();
	}
	HANDLER;
};

#endif