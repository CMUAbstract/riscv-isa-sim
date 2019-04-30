#include "retire.h"

#include "value/core_value.h"

retire_t::retire_t(std::string _name, io::json _config, scheduler_t *_scheduler)
	: module_t("retire", _config, _scheduler), retired_insns("retired_insns") {
	insn_retire_port = create_port<insn_retire_value_t>("insn_retire_port");
	retired_insns.reset();
}

io::json retire_t::to_json() const {
	return io::json::merge_objects(module_t::to_json(), retired_insns);
}