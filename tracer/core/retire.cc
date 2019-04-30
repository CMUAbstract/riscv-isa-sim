#include "retire.h"

#include "event/core_event.h"

retire_t::retire_t(std::string _name, io::json _config, scheduler_t *_scheduler)
	: module_t("retire", _config, _scheduler), retired_insns("retired_insns") {
	insn_retire_port = create_port<signal_port_t<insn_retire_event_t *>>("insn_retire_port");

	register_action(new action_t("retire_retire", [&](){
		if(!insn_retire_port->empty()) {
			insn_retire_port->pop();
			retired_insns.inc();
		}
	}), {"insn_retire_port"}, {});

	retired_insns.reset();
}

io::json retire_t::to_json() const {
	return io::json::merge_objects(module_t::to_json(), retired_insns);
}