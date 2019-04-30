#include "fetch.h"

#include "value/core_value.h"
#include "value/mem_value.h"

fetch_t::fetch_t(
	std::string _name, io::json _config, scheduler_t *_scheduler)
	: module_t("fetch", _config, _scheduler) {
	uint32_t pipeline_depth;
	JSON_CHECK(int, config["depth"], pipeline_depth, 3);
	
	bp_port = create_port<bool>("bp_port");
	insn_fetch_port = create_port<insn_fetch_value_t>("insn_fetch_port");
	insn_decode_port = create_port<insn_decode_value_t>("insn_decode_port");
	insn_squash_port = create_port<insn_squash_value_t>("insn_squash_port");
	mem_read_port = create_port<mem_read_value_t>("mem_read_port");
	mem_retire_port = create_port<mem_retire_value_t>("mem_retire_port");
}

void fetch_t::exec() {
	// Yield control when no squash value has occured and nothing in insn_fetch port
	if(0) scheduler->set_ready(false);
}