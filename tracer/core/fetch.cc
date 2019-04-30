#include "fetch.h"

#include "event/core_event.h"
#include "event/mem_event.h"

fetch_t::fetch_t(
	std::string _name, io::json _config, scheduler_t *_scheduler)
	: module_t("fetch", _config, _scheduler) {
	uint32_t pipeline_depth;
	JSON_CHECK(int, config["depth"], pipeline_depth, 3);
	insns.resize(pipeline_depth);
	
	bp_port = create_port<bool>("bp_port");
	insn_fetch_port = create_port<insn_fetch_event_t *>("insn_fetch_port");
	insn_decode_port = create_port<insn_decode_event_t *>("insn_decode_port");
	insn_squash_port = create_port<insn_squash_event_t *>("insn_squash_port");
	mem_read_port = create_port<mem_read_event_t *>("mem_read_port");
	mem_retire_port = create_port<mem_retire_event_t *>("mem_retire_port");
}

void fetch_t::exec() {
	// Yield control when no squash event has occured and nothing in insn_fetch port
	if(0) scheduler->set_ready(false);
}