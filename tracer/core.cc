#include "core.h"

#include "working_set.h"
#include "core_event.h"
#include "mem_event.h"

core_t::core_t(std::string _name, io::json _config, event_heap_t *_events)
	: component_t(_name, _config, _events), retired_insns("retired_insns", ""),
	running_insns("running_insns", "") {
	pending_handler_t::set_ref(events);
	squash_handler_t::set_ref(events);
	retired_insns.reset();
	running_insns.reset();	
	state["decode"] = false;
	state["exec"] = false;
}

void core_t::reset() {
	component_t::reset();
	clear_pending();
	running_insns.inc(retired_insns.get());
	retired_insns.reset();
	state["decode"] = false;
	state["exec"] = false;
	insns.clear();
}

io::json core_t::to_json() const {
	return io::json::merge_objects(
		component_t::to_json(), retired_insns, running_insns);
}

bool core_t::check_jump(insn_bits_t opc) {
	switch(opc) {
		case MATCH_JALR:
		case MATCH_C_J:
		case MATCH_C_JR:
		case MATCH_C_JAL:
		case MATCH_JAL:
		case MATCH_C_JALR: return true;
	};
	return false;
}