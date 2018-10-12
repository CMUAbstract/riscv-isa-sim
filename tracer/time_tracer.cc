#include "time_tracer.h"

#include <map>
#include <memory>

#include "log.h"
#include "smartptr.h"
#include "working_set.h"
#include "components.h"
#include "insn_event.h"

time_tracer_t::time_tracer_t(io::json _config, elfloader_t *_elf) 
	: tracer_impl_t(_config, _elf) {
	assert_msg(mem_type_map.find(config["mem"]["model"].string_value()) 
		!= mem_type_map.end(), 
		"%s not found", config["mem"]["model"].string_value().c_str());
	mem = mem_type_map.at(config["mem"]["model"].string_value())(
		config["mem"], &events);	
	assert_msg(core_type_map.find(config["core"]["model"].string_value()) 
		!= core_type_map.end(), 
		"%s not found", config["core"]["model"].string_value().c_str());
	core = core_type_map.at(config["core"]["model"].string_value())(
		config["core"], &events, mem);
}

time_tracer_t::~time_tracer_t() {
	delete core;
}

void time_tracer_t::trace(working_set_t *ws, insn_bits_t opc, insn_t insn) {
	auto shared_ws = shared_ptr_t<working_set_t>(new working_set_t(ws));
	auto timed_insn = new timed_insn_t(shared_ws, opc, insn);
	core->buffer_insn(timed_insn);
	while(!events.ready() && !events.empty()) {
		event_base_t *e = events.pop_back();
		e->handle();
		delete e;
	}
}

std::string time_tracer_t::dump() {
	return "";
}