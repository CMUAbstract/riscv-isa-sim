#include "simple_core.h"

#include <common/decode.h>

#include "log.h"
#include "mem.h"
#include "working_set.h"

void simple_core_t::process(insn_event_t<core_t> *event) {
	assert_msg(event->cycle >= clock.get(), "Timing violation");
	clock.set(event->cycle);
	std::cout << "Insn fetch" << std::endl;
	for(auto it : event->ws->input.regs)
		events->push_back(new reg_read_event_t<core_t>(this, it, clock.get()));
	for(auto it : event->ws->output.regs)
		events->push_back(new reg_write_event_t<core_t>(this, it, clock.get()));
	for(auto it : event->ws->input.locs) {
		for(auto child : children) {
			auto mem = dynamic_cast<mem_t *>(child);
			if(mem == nullptr) continue;
			events->push_back(new mem_read_event_t<mem_t>(mem, it, clock.get()));
		}
	}
	for(auto it : event->ws->output.locs) {
		for(auto child : children) {
			auto mem = dynamic_cast<mem_t *>(child);
			if(mem == nullptr) continue;
			events->push_back(new mem_write_event_t<mem_t>(mem, it, clock.get()));
		}
	}
}