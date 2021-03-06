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
	total_energy("total_energy"), total_power("total_power"),
	total_dynamic_energy("total_dynamic_energy"), 
	total_static_energy("total_static_energy"),
	total_dynamic_power("total_dynamic_power"), 
	total_static_power("total_static_power") {

	if(config["intermittent"].is_object()) {
		intermittent = true;
		JSON_CHECK(int, config["intermittent"]["min"], intermittent_min);
		JSON_CHECK(int, config["intermittent"]["max"], intermittent_max);
		
		if(config["intermittent"]["cap_size"].is_array()) {
			auto cap_sizes = config["intermittent"]["cap_size"].array_items();
			assert_msg(cap_sizes.size() == 3, "Must have three cap sizes");
			trace_info.primary.size = cap_sizes[0].double_value();
			trace_info.secondary.size = cap_sizes[1].double_value();
			trace_info.reserve.size = cap_sizes[2].double_value();
		}

		if(config["intermittent"]["esr"].is_array()) {
			auto esrs = config["intermittent"]["esr"].array_items();
			assert_msg(esrs.size() == 3, "Must have three esr values");
			trace_info.primary.esr = esrs[0].double_value();
			trace_info.secondary.esr = esrs[1].double_value();
			trace_info.reserve.esr = esrs[2].double_value();
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
	total_energy.reset();
	total_dynamic_energy.reset();
	total_static_energy.reset();
	total_power.reset();
	total_dynamic_power.reset();
	total_static_power.reset();

	double energy = update_power_energy();

	// Reset caches and wait for failure
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
			recharge_tick();
		}
	}
	// Wait for it to recover
	hard_failures.inc();
	for(auto c : components) c.second->reset(HARD);
	while(!recovered()) recharge_tick();
	for(auto c : components) c.second->reset();
	reset_should_fail();
}

void time_tracer_t::tabulate() {
	core->next_insn();

	bool move_pending = false;
	bool check_pending = false;
	cycle_t pending_cycle = 0;
	hstd::uuid wrapid;
	while(!events.ready() && !events.empty() && 
		(core->get_clock() != TICK_LIMIT || !TICK_LIMIT_ENABLE)) {
		event_base_t *e = events.pop_back();
		
		if(!e->pending) { // Redo pending checks
			// std::cerr << "regular" << std::endl;
			move_pending = false;
			check_pending = false;
		} else if(e->pending && !check_pending && !move_pending) { // Check first pending
			// std::cerr << "starting check: " << e->to_string() << std::endl;
			check_pending = true;
			wrapid = e->id;
		} else if(e->pending && e->id == wrapid && check_pending && !move_pending) { // Move pending to next cycle
			// std::cerr << "starting move: " << e->to_string() << std::endl;
			move_pending = true;
			check_pending = false;
			pending_cycle = e->cycle;
			e->cycle += 1;
			events.push_back(e);
			continue;
		} else if(e->pending && e->cycle == pending_cycle && !check_pending && move_pending) {
			// std::cerr << "doing move: " << e->to_string() << std::endl;
			e->cycle += 1;
			events.push_back(e);
			continue;
		} else if(e->pending && e->cycle != pending_cycle && move_pending && !check_pending) {
			// std::cerr << "done with move" << std::endl;
			move_pending = false;
		}

		if(!e->squashed) e->handle();
		if(e->ready_gc || e->squashed) {
			check_pending = false;
			events.invalidate(e);
			delete e;
		}
	}
	
	update_power_energy();
}

io::json time_tracer_t::to_json() const {
	auto trace = tracer_impl_t::to_json();
	trace[name] = io::json::merge_objects(soft_failures, hard_failures, 
		total_power, total_dynamic_power, total_static_power,
		total_energy, total_dynamic_energy, total_static_energy,
		io::json(components));
	return trace;
}

void time_tracer_t::trace(
	const working_set_t &ws, const insn_bits_t opc, const insn_t &insn) {
	auto shared_timed_insn = hstd::shared_ptr<timed_insn_t>(
		new timed_insn_t(ws, opc, insn));
	
	core->buffer_insn(shared_timed_insn);
	if(hyperdrive_disabled) {
		core->update_pc(ws.pc);
		hyperdrive_disabled = false;
	}

	bool move_pending = false;
	bool check_pending = false;
	cycle_t pending_cycle = 0;
	hstd::uuid wrapid;
	while(!events.ready() && !events.empty() && 
		(core->get_clock() != TICK_LIMIT || !TICK_LIMIT_ENABLE)) {
		event_base_t *e = events.pop_back();
		
		if(!e->pending) { // Redo pending checks
			// std::cerr << "regular" << std::endl;
			move_pending = false;
			check_pending = false;
		} else if(e->pending && !check_pending && !move_pending) { // Check first pending
			// std::cerr << "starting check: " << e->to_string() << std::endl;
			check_pending = true;
			wrapid = e->id;
		} else if(e->pending && e->id == wrapid && check_pending && !move_pending) { // Move pending to next cycle
			// std::cerr << "starting move: " << e->to_string() << std::endl;
			move_pending = true;
			check_pending = false;
			pending_cycle = e->cycle;
			e->cycle += 1;
			events.push_back(e);
			continue;
		} else if(e->pending && e->cycle == pending_cycle && !check_pending && move_pending) {
			// std::cerr << "doing move: " << e->to_string() << std::endl;
			e->cycle += 1;
			events.push_back(e);
			continue;
		} else if(e->pending && e->cycle != pending_cycle && move_pending && !check_pending) {
			// std::cerr << "done with move" << std::endl;
			move_pending = false;
		}

		if(!e->squashed) e->handle();
		if(e->ready_gc || e->squashed) {
			check_pending = false;
			events.invalidate(e);
			delete e;
		}
	}

#if TICK_LIMIT_ENABLE
	if(TICK_LIMIT <= core->get_clock()) exit(0);
#endif

	double energy = update_power_energy();

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

double time_tracer_t::update_power_energy() {
	double static_power = 0., static_energy = 0.;
	double dynamic_power = 0., dynamic_energy = 0.;
	for(auto c : components) {
		static_power += c.second->get_static_power();
		static_energy += c.second->get_static_energy(
			core->get_clock(), core->get_frequency());
	}
	total_static_power.running.set(static_power);
	total_static_energy.running.set(static_energy);

	for(auto c : components) {
		dynamic_energy += c.second->get_dynamic_energy();
		dynamic_power += c.second->get_dynamic_power(core->get_frequency());
	}
	total_dynamic_power.running.set(dynamic_power);
	total_dynamic_energy.running.set(dynamic_energy);

	total_power.running.set(static_power + dynamic_power);
	total_energy.running.set(static_energy + dynamic_energy);

	return static_energy + dynamic_energy;
}