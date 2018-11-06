#ifndef SINGLE_ISSUE_3_STAGE_H
#define SINGLE_ISSUE_3_STAGE_H

#include <set>
#include <sstream>

#include <fesvr/memif.h>

#include "event.h"
#include "core.h"

class ram_t;
class si3stage_core_t: public core_t {
private:
	struct action_set_t {
		std::vector<addr_t> locs;
		bool empty() { return locs.size() == 0; }
	};
	struct pending_event_t: public event_t<si3stage_core_t, action_set_t> {
		using event_t<si3stage_core_t, action_set_t>::event_t;	
		std::string to_string() {
			std::ostringstream os;
			os << "pending_event (" << cycle << ", " << next_event->to_string() << ")"; 
			return os.str();
		}
		HANDLER;
		event_base_t *next_event;
	};
public:
	si3stage_core_t(std::string _name, io::json _config, event_list_t *_events);
	void init();
	void buffer_insn(timed_insn_t *insn);
	void process(insn_fetch_event_t *event);
	void process(insn_decode_event_t *event);
	void process(insn_retire_event_t *event);
	void process(reg_read_event_t *event);
	void process(reg_write_event_t *event);	
	void process(ready_event_t *event);
	void process(stall_event_t *event);
	void process(pending_event_t *event);
private:
	ram_t *icache;
	std::set<pending_event_t *> pending_events;
private:
	void next_insn();
};

#endif