#include "exec.h"

#include "value/core_value.h"
#include "value/mem_value.h"

exec_t::exec_t(std::string _name, io::json _config, scheduler_t *_scheduler)
	: module_t("exec", _config, _scheduler) {
	bp_port = create_port<bool>("bp_port");
	insn_exec_port = create_port<insn_exec_value_t>("insn_exec_port");
	insn_retire_port = create_port<insn_retire_value_t>("insn_retire_port");
	insn_squash_port = create_port<insn_squash_value_t>("insn_squash_port");
	mem_read_port = create_port<mem_read_value_t>("mem_read_port");
	mem_write_port = create_port<mem_write_value_t>("mem_write_port");
	mem_retire_port = create_port<mem_retire_value_t>("mem_retire_port");
	mem_ready_port = create_port<mem_ready_value_t>("mem_ready_port");
	reg_write_port = create_port<reg_write_value_t>("reg_write_port");
}