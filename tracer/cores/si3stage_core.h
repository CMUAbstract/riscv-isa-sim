#ifndef SINGLE_ISSUE_3_STAGE_H
#define SINGLE_ISSUE_3_STAGE_H

#include <set>
#include <sstream>

#include <fesvr/memif.h>

#include "event.h"
#include "core.h"

class ram_t;
class si3stage_core_t: public core_t {
public:
	si3stage_core_t(std::string _name, io::json _config, event_list_t *_events);
	void init();
	io::json to_json() const;
	void buffer_insn(timed_insn_t *insn);
	void process(insn_fetch_event_t *event);
	void process(insn_decode_event_t *event);
	void process(insn_exec_event_t *event);
	void process(insn_retire_event_t *event);
	void process(reg_read_event_t *event);
	void process(reg_write_event_t *event);	
	void process(ready_event_t *event);
	void process(stall_event_t *event);
	void process(pending_event_t *event);
private:
	ram_t *icache;
private:
	void next_insn();
};

#endif