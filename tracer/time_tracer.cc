#include "time_tracer.h"

#include <hstd/memory.h>

#include "log.h"
#include "except.h"
#include "working_set.h"
#include "components.h"

#define TICK_LIMIT_ENABLE 0
#define TICK_LIMIT 60

time_tracer_t::time_tracer_t(io::json _config, elfloader_t *_elf) 
	: tracer_impl_t("time_tracer", _config, _elf), 
	soft_failures("soft_failures"), hard_failures("hard_failures"),
	total_energy("total_energy"), total_power("total_power") {
	if(config["intermittent"].is_object()) {
		intermittent = true;
		JSON_CHECK(int, config["intermittent"]["min"], intermittent_min);
		JSON_CHECK(int, config["intermittent"]["max"], intermittent_max);
		if(config["cap_size"].is_array()) {
			auto cap_sizes = config["cap_size"].array_items();
			assert_msg(cap_sizes.size() == 3, "Must have three cap sizes");
			trace_info.primary.size = cap_sizes[0].double_value();
			trace_info.secondary.size = cap_sizes[1].double_value();
			trace_info.reserve.size = cap_sizes[2].double_value();
		}
		if(config["esr"].is_array()) {
			auto esrs = config["esr"].array_items();
			assert_msg(esrs.size() == 3, "Must have three esr values");
			trace_info.primary.size = esrs[0].double_value();
			trace_info.secondary.size = esrs[1].double_value();
			trace_info.reserve.size = esrs[2].double_value();
			calc_total_esr();
		}
		if(config["intermittent"]["trace"].is_string())
			set_power_trace(config["intermittent"]["trace"].string_value());	
	}
	assert_msg(config["config"].is_object(), "No config provided");
	for(auto it : config["config"].object_items()) {
		assert_msg(it.second.is_object(), "Invalid component config");
		assert_msg(it.second["type"].is_string(), "No type for component");
		if(it.second["type"].string_value().compare("core") == 0) {
			assert_msg(it.second["model"].is_string(), "No core model");
			assert_msg(core_type_map.find(it.second["model"].string_value()) 
				!= core_type_map.end(), "Invalid core model");

			core = core_type_map.at(it.second["model"].string_value())(
				it.first, it.second, &events);
			components.insert({it.first, core});
		} else if(it.second["type"].string_value().compare("mem") == 0) {
			assert_msg(it.second["model"].is_string(), "No mem model");
			assert_msg(ram_type_map.find(it.second["model"].string_value()) 
				!= ram_type_map.end(), "Invalid mem model");

			ram_t *mem = ram_type_map.at(it.second["model"].string_value())(
				it.first, it.second, &events);
			components.insert({it.first, mem});
		} else if(it.second["type"].string_value().compare("vcu") == 0) {
			assert_msg(it.second["model"].is_string(), "No vcu model");
			assert_msg(vcu_type_map.find(it.second["model"].string_value()) 
				!= vcu_type_map.end(), "Invalid vcu model");
			
			vcu_t *vcu = vcu_type_map.at(it.second["model"].string_value())(
				it.first, it.second, &events);
			components.insert({it.first, vcu});
		}
	}
	for(auto it : config["config"].object_items()) {
		if(it.second["children"].is_array()) {
			auto parent_str = it.first;
			for(auto child : it.second["children"].array_items()) {
				auto child_str = child.string_value();
				components[parent_str]->add_child(
					child_str, components[child_str]);
				components[child_str]->add_parent(
					parent_str, components[parent_str]);
			}
		}
	}
	for(auto it : components) it.second->init();
	soft_failures.reset();
	hard_failures.reset();
}

time_tracer_t::~time_tracer_t() {
	dump();
	for(auto it : components) delete it.second;
}

void time_tracer_t::reset(reset_level_t level, uint32_t minstret) {
	events.clear();
	// Reset caches and wait for failure
	double power = 0.;
	for(auto c : components) power += c.second->get_power(component_base_t::BROWN);
	double time = (double)core->get_clock() / (double)core->get_frequency();
	double energy = power * time;
	total_energy.running.inc(energy);
	if(level == SOFT) {
		soft_failures.inc();
		for(auto c : components) c.second->reset(SOFT);
		while(true) {
			if(recovered()) {
				reset_should_fail();
				return;
			}
			if(should_fail(
				core->get_clock(), energy, core->get_frequency())) {
#ifdef INTERMITTENT_LOG
				fprintf(stderr, "Triggering hard failure: Used: %f Stored: %f\n",
					energy, primary_energy + secondary_energy);
#endif
				hard_except_t except;
				except.minstret = core->minstret();
				throw except;
			}
			recharge_tick(core->get_frequency());
		}
	}
	// Wait for it to recover
	hard_failures.inc();
	for(auto c : components) c.second->reset(HARD);
	while(!recovered()) recharge_tick(core->get_frequency());
	for(auto c : components) c.second->reset();
	reset_should_fail();
}

void time_tracer_t::tabulate() {
	core->next_insn();
	while(!events.empty() && !events.ready() &&
		(core->get_clock() != TICK_LIMIT || !TICK_LIMIT_ENABLE)) {
		event_base_t *e = events.pop_back();
		if(!e->squashed) e->handle();
		if(e->ready_gc || e->squashed) {
			events.invalidate(e);
			delete e;
		}
	}
}

io::json time_tracer_t::to_json() const {
	auto trace = tracer_impl_t::to_json();
	trace[name] = io::json::merge_objects(soft_failures, hard_failures, 
		total_power, total_energy, io::json(components));
	return trace;
}

void time_tracer_t::trace(
	const working_set_t &ws, const insn_bits_t opc, const insn_t &insn) {
	auto shared_timed_insn = hstd::shared_ptr<timed_insn_t>(
		new timed_insn_t(ws, opc, insn));
	// std::cerr << "0x" << std::hex << ws.pc << std::endl;
	core->buffer_insn(shared_timed_insn);
	if(hyperdrive_disabled) {
		core->update_pc(ws.pc);
		hyperdrive_disabled = false;
	}
	while(!events.ready() && !events.empty() && 
		(core->get_clock() != TICK_LIMIT || !TICK_LIMIT_ENABLE)) {
		event_base_t *e = events.pop_back();
		if(!e->squashed) e->handle();
		if(e->ready_gc || e->squashed) {
			events.invalidate(e);
			delete e;
		}
	}
#if TICK_LIMIT_ENABLE
	if(TICK_LIMIT <= core->get_clock()) exit(0);
#endif
	double power = 0., energy = 0.;
	for(auto c : components) power += c.second->get_power();
	total_power.running.set(power);
	for(auto c : components) energy += c.second->get_energy();
	double time = (double)core->get_clock() / (double)core->get_frequency();
	energy += power * time;
	total_energy.running.inc(energy);
	if(intermittent && should_fail(
		core->get_clock(), energy, core->get_frequency())) {
#ifdef INTERMITTENT_LOG
		fprintf(stderr, "Triggering soft failure: Used: %f Stored: %f\n",
			energy, primary_energy);
#endif
		// Throw intermittent exception here
		soft_except_t except;
		except.minstret = core->minstret();
		throw except;
	}
}