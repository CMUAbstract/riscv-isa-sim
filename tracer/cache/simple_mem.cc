#include "simple_mem.h"

#include "log.h"
#include "mem_event.h"

void simple_mem_t::process(mem_read_event_t *event) {
	TIME_VIOLATION_CHECK
}

void simple_mem_t::process(mem_write_event_t *event) {
	TIME_VIOLATION_CHECK
}

void simple_mem_t::process(mem_insert_event_t *event) {
	TIME_VIOLATION_CHECK
}

void simple_mem_t::process(mem_ready_event_t *event) {
	TIME_VIOLATION_CHECK
}

void simple_mem_t::process(mem_stall_event_t *event) {
	TIME_VIOLATION_CHECK
}
