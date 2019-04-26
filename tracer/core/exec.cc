#include "exec.h"

#include "core_event.h"
#include "mem_event.h"

exec_t::exec_t(std::string _name, io::json _config, scheduler_t *_scheduler)
	: module_t("exec", _config, _scheduler) {
	bp_port = create_port<persitent_port_t<bool>>("bp_port");
	bp_port->set_default(false);
	insn_exec_port = create_port<signal_port_t<insn_exec_event_t *>>("insn_exec_port");
	insn_retire_port = create_port<signal_port_t<insn_retire_event_t *>>("insn_retire_port");
	mem_read_port = create_port<signal_port_t<mem_read_event_t *>>("mem_read_port");
	mem_write_port = create_port<signal_port_t<mem_write_event_t *>>("mem_write_port");
	mem_retire_port = create_port<signal_port_t<mem_retire_event_t *>>("mem_retire_port");
	mem_ready_port = create_port<signal_port_t<mem_ready_event_t *>>("mem_ready_port");
	reg_write_port = create_port<signal_port_t<reg_write_event_t *>>("reg_write_port");

	assert_msg(1 == 0, "Not yet implemented beyond this point");
	register_action(new action_t("exec", [&](){
		if(!occupied && !insn_exec_port.empty()) {
			cur_insn = insn_exec_port.peek()->data;
			insn_exec_port.pop();
		}
	}));
}