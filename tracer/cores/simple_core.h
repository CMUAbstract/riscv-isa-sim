#ifndef SIMPLE_CORE_H
#define SIMPLE_CORE_H

#include "core.h"
#include "mem.h"

class simple_core_t: public core_t {
public:
	using core_t::core_t;
	void process(insn_fetch_event_t<core_t> *event);
	void process(reg_read_event_t<core_t> *event) {}
	void process(reg_write_event_t<core_t> *event) {}
};

#endif