#ifndef CORE_HANDLER_H
#define CORE_HANDLER_H

#include "handler.h"

struct insn_fetch_event_t;
struct insn_decode_event_t;
struct insn_exec_event_t;
struct insn_retire_event_t;
struct reg_read_event_t;
struct reg_write_event_t;
class core_handler_t: 
	public handler_t<insn_fetch_event_t *>, public handler_t<insn_decode_event_t *>, 
	public handler_t<insn_exec_event_t *>, public handler_t<insn_retire_event_t *>, 
	public handler_t<reg_read_event_t *>, public handler_t<reg_write_event_t *> {
public:
	using handler_t<insn_fetch_event_t *>::process;
	using handler_t<insn_decode_event_t *>::process;
	using handler_t<insn_exec_event_t *>::process;
	using handler_t<insn_retire_event_t *>::process;
	using handler_t<reg_read_event_t *>::process;
	using handler_t<reg_write_event_t *>::process;
};

#endif