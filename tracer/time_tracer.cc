#include "time_tracer.h"

#include "log.h"
#include "smartptr.h"
#include "except.h"
#include "working_set.h"
#include "components.h"
#include "mem_event.h"
#include "core_event.h"
#include "signal_event.h"

#define EVENT_LIMIT_ENABLE 0
#define EVENT_LIMIT 10

time_tracer_t::time_tracer_t(io::json _config, elfloader_t *_elf) 
	: tracer_impl_t("time_tracer", _config, _elf) {
	if(config["intermittent"].is_object()) {
		intermittent = true;
		JSON_CHECK(int, config["intermittent"]["max"], intermittent_min);
		JSON_CHECK(int, config["intermittent"]["max"], intermittent_max);
	}
	assert_msg(config["config"].is_array(), "No config provided");
	for(auto it : config["config"].array_items()) {
		assert_msg(it.is_object(), "Invalid component config");
		assert_msg(it["type"].is_string(), "No type for component");
		if(it["type"].string_value().compare("core") == 0) {
			assert_msg(it["model"].is_string(), "No core model");
			assert_msg(it["name"].is_string(), "No name");
			assert_msg(core_type_map.find(it["model"].string_value()) 
				!= core_type_map.end(), "Invalid core model");
			core = core_type_map.at(it["model"].string_value())(
				it["name"].string_value(), it, &events);
			components.insert({it["name"].string_value(), core});
		} else if(it["type"].string_value().compare("mem") == 0) {
			assert_msg(it["model"].is_string(), "No mem model");
			assert_msg(it["name"].is_string(), "No name");
			assert_msg(ram_type_map.find(it["model"].string_value()) 
				!= ram_type_map.end(), "Invalid mem model");
			ram_t *mem = ram_type_map.at(it["model"].string_value())(
				it["name"].string_value(), it, &events);
			components.insert({it["name"].string_value(), mem});
		}
	}
	for(auto it : config["config"].array_items()) {
		if(it["children"].is_array()) {
			auto parent_str = it["name"].string_value();
			for(auto child : it["children"].array_items()) {
				auto child_str = child.string_value();
				components[parent_str]->add_child(
					child_str, components[child_str]);
				components[child_str]->add_parent(
					parent_str, components[parent_str]);
			}	
		}
	}
	for(auto it : components) it.second->init();
}

time_tracer_t::~time_tracer_t() {
	dump();
	for(auto it : components) delete it.second;
}

void time_tracer_t::reset(size_t minstret) {
	events.clear();
	for(auto c : components) c.second->reset();
}

io::json time_tracer_t::to_json() const {
	std::vector<component_t *> comps;
	for(auto it : components) comps.push_back(it.second);
	auto trace = tracer_impl_t::to_json();
	trace[name] = io::json(comps);
	return trace;
}

void time_tracer_t::trace(working_set_t *ws, insn_bits_t opc, insn_t insn) {
	auto shared_ws = shared_ptr_t<working_set_t>(new working_set_t(ws));
	auto timed_insn = new timed_insn_t(shared_ws, opc, insn);
	core->buffer_insn(timed_insn);
	uint32_t event_limit = EVENT_LIMIT;
	while(!events.ready() && !events.empty() && 
		(event_limit-- || !EVENT_LIMIT_ENABLE)) {
		event_base_t *e = events.pop_back();
		e->handle();
		if(e->ready_gc) delete e;
	}
	if(intermittent && should_fail(core->get_clock())) {
#if 1
		std::cout << "Triggering intermittent failure" << std::endl;
#endif
		// Throw intermittent exception here
		intermittent_except_t except;
		except.minstret = core->minstret();
		throw except;
	}
}