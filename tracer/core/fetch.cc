#include "fetch.h"

#include "event/core_event.h"
#include "event/mem_event.h"

fetch_t::fetch_t(
	std::string _name, io::json _config, scheduler_t *_scheduler)
	: module_t("fetch", _config, _scheduler) {
	
	bp_port = create_port<persistent_port_t<bool>>("bp");
	bp_port->set_default(false);
	insn_fetch_port = create_port<signal_port_t<insn_fetch_event_t *>>("insn_fetch_port");
	insn_decode_port = create_port<signal_port_t<insn_decode_event_t *>>("insn_decode_port");
	mem_read_port = create_port<signal_port_t<mem_read_event_t *>>("mem_read_port");
	mem_retire_port = create_port<signal_port_t<mem_retire_event_t *>>("mem_retire_port");
	
	register_action(new action_t("fetch", [&](){
		if(!bp_port->peek() && insn_fetch_port->size() && !occupied) {
			occupied = true;
			cur_insn = insn_fetch_port->peek()->data;
			insn_fetch_port->pop();
			// mem_read_port->push(new mem_read_event_t(cur_insn));
			// pc += 4;
		}
	}));

	register_action(new action_t("decode", [&]() {
		if(mem_retire_port->size() && occupied) {
			occupied = false;
			mem_retire_port->pop();
			insn_decode_port->push(new insn_decode_event_t(cur_insn));
		}
	}));
}