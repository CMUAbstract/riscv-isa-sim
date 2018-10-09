#include "time_tracer.h"

#include <map>

#include "log.h"

#include "core.h"
#include "cores/simple_core.h"

#include "mem.h"

template<typename T> core_t* create_core(io::json config, event_list_t *events) {
	return new T(config, events);
}

template<typename T> mem_t *create_mem(io::json config, event_list_t *events) {
	return new T(config, events);
}

std::map<std::string, core_t*(*)(io::json, event_list_t *)> core_type_map = {
	{"simple_core", &create_core<simple_core_t>}
};

std::map<std::string, mem_t*(*)(io::json, event_list_t *)> mem_type_map = {

};

time_tracer_t::time_tracer_t(io::json _config, elfloader_t *_elf) 
	: tracer_impl_t(_config, _elf) {
	assert_msg(core_type_map.find(config["core"]["model"].string_value()) 
			!= core_type_map.end(), 
			"%s not found", config["core"]["model"].string_value().c_str());
	core = core_type_map[config["core"]["model"].string_value()](
		config["core"], &events);
}

time_tracer_t::~time_tracer_t() {
	delete core;
}

void time_tracer_t::trace(working_set_t *ws, insn_bits_t opc, insn_t insn) {
	auto insn_event = new insn_event_t<core_t>(core, ws, opc, insn);
	events.push_back(insn_event);
	while(!events.ready() && !events.empty()) {
		event_base_t *e = events.pop_back();
		e->handle();
		delete e;
	}
}

std::string time_tracer_t::dump() {
	return "";
}