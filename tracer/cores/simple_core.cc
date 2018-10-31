#include "simple_core.h"

#include <common/decode.h>

#include "log.h"
#include "mem.h"
#include "working_set.h"
#include "core_event.h"
#include "mem_event.h"

void simple_core_t::process(insn_fetch_event_t *event) {
	assert_msg(event->cycle >= clock.get(), "Timing violation");
	clock.set(event->cycle);
	for(auto it : event->data.ws->input.regs)
		events->push_back(new reg_read_event_t(this, it, clock.get()));
	for(auto it : event->data.ws->output.regs)
		events->push_back(new reg_write_event_t(this, it, clock.get()));
	for(auto it : event->data.ws->input.locs) {
		for(auto child : children) {
			auto mem = dynamic_cast<mem_t *>(child.second);
			if(mem == nullptr) continue;
			events->push_back(new mem_read_event_t(mem, it, clock.get()));
		}
	}
	for(auto it : event->data.ws->output.locs) {
		for(auto child : children) {
			auto mem = dynamic_cast<mem_t *>(child.second);
			if(mem == nullptr) continue;
			events->push_back(new mem_write_event_t(mem, it, clock.get()));
		}
	}
}