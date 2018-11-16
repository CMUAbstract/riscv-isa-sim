#include "core.h"

#include "working_set.h"
#include "core_event.h"
#include "mem_event.h"

core_t::core_t(std::string _name, io::json _config, event_list_t *_events)
	: component_t(_name, _config, _events), retired_insns("retired_insns", ""),
	running_insns("running_insns", "") {
	retired_insns.reset();
	running_insns.reset();	
	status["decode"] = false;
	status["exec"] = false;
}

core_t::~core_t() {
	for(auto it : insns) delete it;
}

void core_t::reset() {
	running_insns.inc(retired_insns.get());
	retired_insns.reset();
}

io::json core_t::to_json() const {
	return io::json::merge_objects(
		component_t::to_json(), retired_insns, running_insns);
}

void core_t::buffer_insn(timed_insn_t *insn) {
	insns.push_back(insn);
	auto i = new insn_fetch_event_t(this, insns.front());
	clock.inc();
	i->cycle = clock.get();
	insns.erase(insns.begin());
	events->push_back(i);
}