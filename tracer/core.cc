#include "core.h"

#include "working_set.h"
#include "core_event.h"
#include "mem_event.h"

core_t::core_t(std::string _name, io::json _config, event_hmap_t *_events)
	: component_t(_name, _config, _events), retired_insns("retired_insns", ""),
	running_insns("running_insns", "") {
	retired_insns.reset();
	running_insns.reset();	
	squashed_events.set_ref(events);
	pending_events.set_ref(events);
	state["fetch"].issued.set_ref(_events);
	state["decode"].issued.set_ref(_events);
	state["decode"].status = false;
	state["exec"].status = false;
}

void core_t::reset() {
	component_t::reset();
	clear_pending();
	running_insns.inc(retired_insns.get());
	retired_insns.reset();
	state["decode"].status = false;
	state["exec"].status = false;
	insns.clear();
}

io::json core_t::to_json() const {
	return io::json::merge_objects(
		component_t::to_json(), retired_insns, running_insns);
}