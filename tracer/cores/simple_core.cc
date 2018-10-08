#include "simple_core.h"

#include <common/decode.h>

#include "mem.h"
#include "working_set.h"

void simple_core_t::process(insn_event_t event) {
	for(auto it : event.ws->input.regs)
		events->push_back(reg_read_event_t(it, cycle));
	for(auto it : event.ws->output.regs)
		events->push_back(reg_write_event_t(it, cycle));
	for(auto it : event.ws->input.locs)
		events->push_back(mem_read_event_t(it, cycle));
	for(auto it : event.ws->output.locs)
		events->push_back(mem_write_event_t(it, cycle));
}