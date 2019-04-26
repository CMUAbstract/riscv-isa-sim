#ifndef MEM_EVENT_H
#define MEM_EVENT_H

#include <sstream>

#include "helpers.h"
#include "event.h"

struct reg_event_t: public event_t<reg_t> {
	using event_t::event_t;
	io::to_json() const { 
		return {
			{get_name(), {
					{"data", data},
					{"cycle", cycle}
				}
			}
		}; 
	}
	std::string to_string() {
		std::ostringstream os;
		os << get_name() << " (" << cycle << "," << data << ")"; 
		return os.str();
	}
};

struct reg_read_event_t : public reg_event_t {
	using reg_event_t::reg_event_t;
	std::string get_name() { return "reg_read_event"}
}

struct reg_write_event_t : public reg_event_t {
	using reg_event_t::reg_event_t;
	std::string get_name() { return "reg_write_event"}
}

struct mem_event_info_t {
	addr_t addr;
	bool reader = false;
};

struct mem_event_t : public event_t<mem_event_info_t> {
	using event_t::event_t;
	io::to_json() const {
		return return {
			{get_name(), {
					{"addr", hexify(data.addr)},
					{"cycle", cycle}
				}
			}
		}; 
	}
	std::string to_string() {
		std::ostringstream os;
		os << get_name() << " (" << cycle << ", 0x";
		os << std::hex << data.addr << ")"; 
		return os.str();
	}
}

struct mem_read_event_t: public mem_event_t {
	using mem_event_t::mem_event_t;
	std::string get_name() { return "mem_read_event"; }
};

struct mem_write_event_t: public mem_event_t {
	using mem_event_t::mem_event_t;
	std::string get_name() { return "mem_write_event"; }
};

struct mem_insert_event_t: public mem_event_t {
	using mem_event_t::mem_event_t;
	std::string get_name() { return "mem_insert_event"; }
};

struct mem_ready_event_t: public mem_event_t{
	using mem_event_t::mem_event_t;
	std::string get_name() { return "mem_ready_event"; }
};

struct mem_retire_event_t: public mem_event_t{
	using mem_event_t::mem_event_t;
	std::string get_name() { return "mem_ready_event"; }
};

#endif