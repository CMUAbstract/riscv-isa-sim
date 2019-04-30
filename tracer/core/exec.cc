#include "exec.h"

#include "event/core_event.h"
#include "event/mem_event.h"

exec_t::exec_t(std::string _name, io::json _config, scheduler_t *_scheduler)
	: module_t("exec", _config, _scheduler) {
	bp_port = create_port<bool>("bp_port");
	insn_exec_port = create_port<insn_exec_event_t *>("insn_exec_port");
	insn_retire_port = create_port<insn_retire_event_t *>("insn_retire_port");
	insn_squash_port = create_port<insn_squash_event_t *>("insn_squash_port");
	mem_read_port = create_port<mem_read_event_t *>("mem_read_port");
	mem_write_port = create_port<mem_write_event_t *>("mem_write_port");
	mem_retire_port = create_port<mem_retire_event_t *>("mem_retire_port");
	mem_ready_port = create_port<mem_ready_event_t *>("mem_ready_port");
	reg_write_port = create_port<reg_write_event_t *>("reg_write_port");
}