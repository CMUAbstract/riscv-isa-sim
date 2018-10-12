#include "simple_mem.h"

#include "log.h"
#include "mem_event.h"

void simple_mem_t::process(mem_read_event_t *event) {
	assert_msg(event->cycle >= clock.get(), "Timing violation");
	clock.set(event->cycle);
}

void simple_mem_t::process(mem_write_event_t *event) {
	assert_msg(event->cycle >= clock.get(), "Timing violation");
	clock.set(event->cycle);
}

void simple_mem_t::process(mem_miss_event_t *event) {
	assert_msg(event->cycle >= clock.get(), "Timing violation");
	clock.set(event->cycle);
}

void simple_mem_t::process(mem_invalidate_event_t *event) {
	assert_msg(event->cycle >= clock.get(), "Timing violation");
	clock.set(event->cycle);
}
