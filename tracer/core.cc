#include "core.h"

#include "log.h"
#include "working_set.h"

#include "cache/simple_mem.h"

static std::map<std::string, mem_t*(*)(io::json, event_list_t *)> mem_type_map = {
	{"simple_mem", &create_mem<simple_mem_t>}
};

core_t::core_t(io::json _config, event_list_t *_events, mem_t *_mm)
	: component_t(_config, _events), mm(_mm) {
	assert_msg(mem_type_map.find(config["mem"]["model"].string_value()) 
		!= mem_type_map.end(), 
		"%s not found", config["mem"]["model"].string_value().c_str());
	auto mem = mem_type_map[config["mem"]["model"].string_value()](
		config["mem"], events);
	add_child(mem);	
	mem->add_child(mm);
}

core_t::~core_t() {
	for(auto it : insns) {
		delete it->ws;
		delete it;
	}
}

void core_t::buffer_insn(timed_insn_t *insn) {
	insns.push_back(insn);
	auto i = new insn_fetch_event_t<core_t>(this, insns.front());
	i->cycle = clock.get() + 1;
	insns.erase(insns.begin());
	events->push_back(i);
}