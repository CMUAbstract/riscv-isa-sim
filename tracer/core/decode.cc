#include "decode.h"

#include "event/core_event.h"

decode_t::decode_t(std::string _name, io::json _config, scheduler_t *_scheduler)
	: module_t("decode", _config, _scheduler) {
	bp_input_port = create_port<persistent_port<bool>>("bp_input_port");
	bp_output_port = create_port<persistent_port<bool>>("bp_output_port");
	bp_output_port->set_default(false);
	squash_input_port = create_port<persistent_port<bool>>("squash_input_port");
	squash_output_port = create_port<persistent_port<bool>>("squash_output_port");
	squash_output_port->set_default(false);
	insn_decode_port = create_port<signal_port_t<insn_decode_event_t *>>("insn_decode_port");
	insn_exec_port = create_port<signal_port_t<insn_exec_event_t *>>("insn_exec_port");
	reg_read_port = create_port<signal_port_t<reg_read_event_t *>>("reg_read_port");

	assert_msg(1 == 0, "Not yet implemented beyond this point");
}