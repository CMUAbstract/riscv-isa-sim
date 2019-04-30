#include "fetch.h"

#include "event/core_event.h"
#include "event/mem_event.h"

fetch_t::fetch_t(
	std::string _name, io::json _config, scheduler_t *_scheduler)
	: module_t("fetch", _config, _scheduler) {
	uint32_t pipeline_depth;
	JSON_CHECK(int, config["depth"], pipeline_depth, 3);
	insns.resize(pipeline_depth);
	
	bp_port = create_port<persistent_port_t<bool>>("bp_port");
	bp_port->set_default(false);
	insn_fetch_port = create_port<signal_port_t<insn_fetch_event_t *>>("insn_fetch_port");
	insn_decode_port = create_port<signal_port_t<insn_decode_event_t *>>("insn_decode_port");
	insn_squash_port = create_port<signal_port_t<insn_squash_event_t *>>("insn_squash_port");
	mem_read_port = create_port<signal_port_t<mem_read_event_t *>>("mem_read_port");
	mem_retire_port = create_port<signal_port_t<mem_retire_event_t *>>("mem_retire_port");
	
	register_action(new action_t("fetch", [&](){
		if(!bp_port->peek() && !insn_fetch_port->empty() && !occupied) {
			occupied = true;
			insns[idx] = insn_fetch_port->peek()->data;
			insns[idx]->idx = idx;
			insn_fetch_port->pop();
			mem_read_port->push(new mem_read_event_t({.addr=insns[idx]->ws.pc}));
		}
	}), {"bp_port", "insn_fetch_port"}, {"mem_read_port"});

	register_action(new action_t("decode", [&]() {
		if(!mem_retire_port->empty() && occupied) {
			auto addr = mem_retire_port->peek()->data.addr;
			if(addr == insns[idx]->ws.pc) {
				occupied = false;
				mem_retire_port->pop();
				insn_decode_port->push(new insn_decode_event_t(insns[idx]));
				idx = (idx + 1) % insns.size();
			}
		}
	}), {"mem_retire_port"}, {"insn_decode_port"});

	register_action(new action_t("squash", [&] {
		if(!insn_squash_port->empty()) {
			occupied = true;
			idx = (insn_squash_port->peek()->data->idx + 1) % insns.size();
			insn_squash_port->pop();
			mem_read_port->push(new mem_read_event_t({.addr=insns[idx]->ws.pc}));
		}
	}), {"insn_squash_port"}, {"mem_read_port"});
}