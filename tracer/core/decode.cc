#include "decode.h"

#include "event/core_event.h"
#include "event/mem_event.h"

decode_t::decode_t(std::string _name, io::json _config, scheduler_t *_scheduler)
	: module_t("decode", _config, _scheduler) {
	bp_input_port = create_port<bool>("bp_input_port");
	bp_output_port = create_port<bool>("bp_output_port");
	insn_decode_port = create_port<insn_decode_event_t *>("insn_decode_port");
	insn_exec_port = create_port<insn_exec_event_t *>("insn_exec_port");
	insn_squash_input_port = create_port<insn_squash_event_t *>("insn_squash_input_port");
	insn_squash_output_port = create_port<insn_squash_event_t *>("insn_squash_output_port");
	reg_read_port = create_port<reg_read_event_t *>("reg_read_port");
}