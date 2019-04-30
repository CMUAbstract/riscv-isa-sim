#ifndef MEM_EVENT_H
#define MEM_EVENT_H

#include <sstream>
#include <map>

#include <fesvr/memif.h>

#include "helpers.h"
#include "value.h"

struct reg_value_t: public value_t<reg_t> {
	using value_t<reg_t>::value_t;
	io::json to_json() const { 
		return std::map<std::string, reg_t> {{get_name(), *data}}; 
	}
	std::string to_string() const {
		std::ostringstream os;
		os << get_name() << " (" << *data << ")"; 
		return os.str();
	}
};

struct reg_read_value_t : public reg_value_t {
	using reg_value_t::reg_value_t;
	std::string get_name() const { return "reg_read_value"; }
};

struct reg_write_value_t : public reg_value_t {
	using reg_value_t::reg_value_t;
	std::string get_name() const { return "reg_write_value"; }
};

struct mem_value_info_t {
	addr_t addr;
	bool reader = false;
};

struct mem_value_t : public value_t<mem_value_info_t> {
	using value_t<mem_value_info_t>::value_t;
	io::json to_json() const {
		return std::map<std::string, std::string>{
			{get_name(), hexify(data->addr)}
		}; 
	}
	std::string to_string() const {
		std::ostringstream os;
		os << get_name() << " (0x" << std::hex << data->addr << ")"; 
		return os.str();
	}
};

struct mem_read_value_t: public mem_value_t {
	using mem_value_t::mem_value_t;
	std::string get_name() const { return "mem_read_value"; }
};

struct mem_write_value_t: public mem_value_t {
	using mem_value_t::mem_value_t;
	std::string get_name() const { return "mem_write_value"; }
};

struct mem_insert_value_t: public mem_value_t {
	using mem_value_t::mem_value_t;
	std::string get_name() const { return "mem_insert_value"; }
};

struct mem_ready_value_t: public mem_value_t{
	using mem_value_t::mem_value_t;
	std::string get_name() const { return "mem_ready_value"; }
};

struct mem_retire_value_t: public mem_value_t{
	using mem_value_t::mem_value_t;
	std::string get_name() const { return "mem_ready_value"; }
};

#endif