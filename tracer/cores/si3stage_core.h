#ifndef SINGLE_ISSUE_3_STAGE_H
#define SINGLE_ISSUE_3_STAGE_H

#include "core.h"
#include "mem.h"

class si3stage_core_t: public core_t {
public:
	si3stage_core_t(io::json _config, event_list_t *_events, mem_t *_mm);
	void buffer_insn(timed_insn_t *insn);
	void process(stall_event_t *event);
	void process(insn_fetch_event_t *event);
	void process(insn_decode_event_t *event);
	void process(reg_read_event_t *event);
	void process(reg_write_event_t *event);	
private:
	mem_t *insn_cache;
};

#endif