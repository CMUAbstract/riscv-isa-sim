#ifndef CORE_EVENT_H
#define CORE_EVENT_H

#include <sstream>

#include <common/decode.h>
#include <hstd/memory.h>

#include "helpers.h"
#include "event.h"
#include "insn_info.h"

struct timed_insn_t;
template<class T>
struct insn_event_t: public event_t<T, hstd::shared_ptr<insn_info_t>> {
	using event_t<T, hstd::shared_ptr<timed_insn_t>>::event_t;
	
	io::json to_json() const {
		return {
			{get_name(), {
					{"cycle", cycle},
					{"pc": hexify(data->ws.pc)},
					{"idx": data->idx}
				}
			}
		};
	}

	std::string to_string() {
		std::ostringstream os;
		os << " (" << cycle << ", 0x" << std::hex << data->ws.pc;
		os << ", " << data->idx << ")"; 
		return os.str();
	}
};

struct insn_fetch_event_t: public insn_event_t {
	using insn_event_t::insn_event_t;
	std::string get_name() { return "insn_fetch_event"; }
};

struct insn_decode_event_t: public insn_event_t {
	using insn_event_t::insn_event_t;
	std::string get_name() { return "insn_decode_event"; }
};

struct insn_exec_event_t: public insn_event_t {
	using insn_event_t::insn_event_t;
	std::string get_name() { return "insn_exec_event"; }
};

struct insn_retire_event_t: public insn_event_t {
	using insn_event_t::insn_event_t;
	std::string get_name() { return "insn_retire_event"; }
};

#endif