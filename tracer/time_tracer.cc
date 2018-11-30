#include "time_tracer.h"

#include "log.h"
#include "smartptr.h"
#include "except.h"
#include "working_set.h"
#include "components.h"
#include "mem_event.h"
#include "core_event.h"
#include "signal_event.h"

#define TICK_LIMIT_ENABLE 1
#define TICK_LIMIT 40

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
	fail();
	for(auto c : components) c.second->reset();
}

io::json time_tracer_t::to_json() const {
	auto trace = tracer_impl_t::to_json();
	trace[name] = io::json(components);
	return trace;
}

void time_tracer_t::trace(
	const working_set_t &ws, const insn_bits_t opc, const insn_t &insn) {
	auto shared_timed_insn = shared_ptr_t<timed_insn_t>(
		new timed_insn_t(ws, opc, insn));
	core->buffer_insn(shared_timed_insn);
	bool ran = false;
	while(!events.ready() && !events.empty() && 
		(core->get_clock() != TICK_LIMIT || !TICK_LIMIT_ENABLE)) {
		ran = true;
		event_base_t *e = events.pop_back();
		if(!e->squashed) e->handle();
		if(e->ready_gc || e->squashed) {
			events.invalidate(e);
			delete e;
		}
	}
#if TICK_LIMIT_ENABLE
	if(TICK_LIMIT == core->get_clock()) exit(0);
#endif
	if(intermittent && should_fail(core->get_clock())) {
#if 1
		std::cout << std::endl << "Triggering intermittent failure" << std::endl;
#endif
		// Throw intermittent exception here
		intermittent_except_t except;
		except.minstret = core->minstret();
		throw except;
	}
}