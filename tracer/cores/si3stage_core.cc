#include "si3stage_core.h"

#include "log.h"
#include "mem.h"
#include "working_set.h"
#include "insn_event.h"
#include "mem_event.h"

si3stage_core_t::si3stage_core_t(io::json _config, event_list_t *_events, mem_t *_mm)
	: core_t(_config, _events, _mm) {

}

void si3stage_core_t::buffer_insn(timed_insn_t *insn) {

}

void si3stage_core_t::process(insn_fetch_event_t *event) {

}

void si3stage_core_t::process(insn_decode_event_t *event) {

}

void si3stage_core_t::process(reg_read_event_t *event) {

}

void si3stage_core_t::process(reg_write_event_t *event) {

}
