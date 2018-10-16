#include "core.h"

#include "log.h"
#include "working_set.h"
#include "components.h"
#include "core_event.h"

core_t::core_t(io::json _config, event_list_t *_events, mem_t *_mm)
	: component_t(_config, _events), mm(_mm) {
	assert_msg(mem_type_map.find(config["mem"]["model"].string_value()) 
		!= mem_type_map.end(), 
		"%s not found", config["mem"]["model"].string_value().c_str());
	auto mem = mem_type_map.at(config["mem"]["model"].string_value())(
		config["mem"], events);
	add_child(mem);	
	mem->add_child(mm);
}

core_t::~core_t() {
	for(auto it : insns) {
		delete it;
	}
}

void core_t::buffer_insn(timed_insn_t *insn) {
	insns.push_back(insn);
	auto i = new insn_fetch_event_t(this, insns.front());
	clock.inc();
	i->cycle = clock.get();
	insns.erase(insns.begin());
	events->push_back(i);
}