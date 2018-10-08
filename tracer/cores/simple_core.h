#ifndef SIMPLE_CORE_H
#define SIMPLE_CORE_H

#include "core.h"

class simple_core_t: public core_t {
public:
	using core_t::core_t;
	void process(insn_event_t event);
};

#endif