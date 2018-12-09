#ifndef CORE_HANDLER_H
#define CORE_HANDLER_H

#include <unordered_map>

struct insn_fetch_event_t;
struct insn_decode_event_t;
struct insn_exec_event_t;
struct insn_retire_event_t;
struct reg_read_event_t;
struct reg_write_event_t;
class core_handler_t {
public:
	virtual void process(insn_fetch_event_t *) = 0;
	virtual void process(insn_decode_event_t *) = 0;
	virtual void process(insn_exec_event_t *) = 0;
	virtual void process(insn_retire_event_t *) = 0;
	virtual void process(reg_read_event_t *) = 0;
	virtual void process(reg_write_event_t *) = 0;
};

#endif