#ifndef CORE_EVENT_H
#define CORE_EVENT_H

#include <sstream>

#include <common/decode.h>
#include <hstd/memory.h>

#include "helpers.h"
#include "event.h"
#include "insn_info.h"

struct insn_event_t : public event_t<hstd::shared_ptr<insn_info_t>> {
	using event_t<hstd::shared_ptr<insn_info_t>>::event_t;
	
	io::json to_json() const {
		return std::map<std::string, std::map<std::string, std::string>>{
			{get_name(), {
					{"cycle", std::to_string(cycle)},
					{"pc", hexify(data->ws.pc)},
					{"idx", std::to_string(data->idx)}
				}
			}
		};
	}

	std::string to_string() const {
		std::ostringstream os;
		os << get_name() << " (" << cycle;
		os << ", 0x" << std::hex << data->ws.pc;
		os << ", " << data->idx << ")"; 
		return os.str();
	}
};

struct insn_fetch_event_t : public insn_event_t {
	using insn_event_t::insn_event_t;
	std::string get_name() const { return "insn_fetch_event"; }
};

struct insn_decode_event_t : public insn_event_t {
	using insn_event_t::insn_event_t;
	std::string get_name() const { return "insn_decode_event"; }
};

struct insn_exec_event_t : public insn_event_t {
	using insn_event_t::insn_event_t;
	std::string get_name() const { return "insn_exec_event"; }
};

struct insn_retire_event_t : public insn_event_t {
	using insn_event_t::insn_event_t;
	std::string get_name() const { return "insn_retire_event"; }
};

struct insn_squash_event_t : public insn_event_t {
	using insn_event_t::insn_event_t;
	std::string get_name() const { return "insn_squash_event"; }
};

struct predict_event_t : public insn_event_t {
	using insn_event_t::insn_event_t;
	std::string get_name() const { return "predict_event"; }
};

struct check_predict_event_t : public insn_event_t {
	using insn_event_t::insn_event_t;
	std::string get_name() const { return "check_predict_event"; }
};

struct branch_event_t : public event_t<bool> {
	using event_t<bool>::event_t;

	io::json to_json() const {
		return data;
	}

	std::string to_string() const {
		std::ostringstream os;
		os << get_name() << "(" << data<< ")";
		return os.str();
	}

	std::string get_name() const { return "branch_event"; }
};

#endif