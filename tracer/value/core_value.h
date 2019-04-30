#ifndef CORE_EVENT_H
#define CORE_EVENT_H

#include <sstream>

#include <common/decode.h>
#include <hstd/memory.h>

#include "helpers.h"
#include "value.h"
#include "insn_info.h"

struct insn_value_t : public value_t<insn_info_t> {
	using value_t<insn_info_t>::value_t;
	
	io::json to_json() const {
		return std::map<std::string, std::map<std::string, std::string>>{
			{get_name(), {
					{"pc", hexify(data->ws.pc)},
					{"idx", std::to_string(data->idx)}
				}
			}
		};
	}

	std::string to_string() const {
		std::ostringstream os;
		os << get_name() << " (";
		os << "0x" << std::hex << data->ws.pc;
		os << ", " << data->idx << ")"; 
		return os.str();
	}
};

struct insn_fetch_value_t : public insn_value_t {
	using insn_value_t::insn_value_t;
	std::string get_name() const { return "insn_fetch_value"; }
};

struct insn_decode_value_t : public insn_value_t {
	using insn_value_t::insn_value_t;
	std::string get_name() const { return "insn_decode_value"; }
};

struct insn_exec_value_t : public insn_value_t {
	using insn_value_t::insn_value_t;
	std::string get_name() const { return "insn_exec_value"; }
};

struct insn_retire_value_t : public insn_value_t {
	using insn_value_t::insn_value_t;
	std::string get_name() const { return "insn_retire_value"; }
};

struct insn_squash_value_t : public insn_value_t {
	using insn_value_t::insn_value_t;
	std::string get_name() const { return "insn_squash_value"; }
};

struct predict_value_t : public insn_value_t {
	using insn_value_t::insn_value_t;
	std::string get_name() const { return "predict_value"; }
};

struct check_predict_value_t : public insn_value_t {
	using insn_value_t::insn_value_t;
	std::string get_name() const { return "check_predict_value"; }
};

struct branch_value_t : public value_t<bool> {
	using value_t<bool>::value_t;

	io::json to_json() const {
		return *data;
	}

	std::string to_string() const {
		std::ostringstream os;
		os << get_name() << "(" << *data << ")";
		return os.str();
	}

	std::string get_name() const { return "branch_value"; }
};

#endif