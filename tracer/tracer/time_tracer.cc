#include "time_tracer.h"

#include <hstd/memory.h>

#include "log.h"
#include "except.h"
#include "insn_info.h"
#include "working_set.h"

#define TICK_LIMIT_ENABLE 0
#define TICK_LIMIT 60

time_tracer_t::time_tracer_t(io::json _config, elfloader_t *_elf) 
	: tracer_impl_t("time_tracer", _config, _elf), 
	modules("top", config["config"], &scheduler),
	soft_failures("soft_failures"), hard_failures("hard_failures"),
	total_energy("total_energy"), total_power("total_power"),
	total_dynamic_energy("total_dynamic_energy"), 
	total_static_energy("total_static_energy"),
	total_dynamic_power("total_dynamic_power"), 
	total_static_power("total_static_power") {

	assert_msg(config["config"].is_object(), "No config provided");

	scheduler.schedule_actions();
	
	#if 0
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
	
	soft_failures.reset();
	hard_failures.reset();
	#endif
}

time_tracer_t::~time_tracer_t() {
	dump();
}

void time_tracer_t::reset(reset_level_t level, uint32_t minstret) {
	scheduler.reset();
	total_energy.reset();
	total_dynamic_energy.reset();
	total_static_energy.reset();
	total_power.reset();
	total_dynamic_power.reset();
	total_static_power.reset();

#if 0

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
#endif
}

void time_tracer_t::tabulate() {
#if 0
	core->next_insn();

	while(scheduler.ready() && !scheduler.empty() && 
		(core->get_clock() != TICK_LIMIT || !TICK_LIMIT_ENABLE)) {
		scheduler.tick();
	}

	update_power_energy();
#endif
}

io::json time_tracer_t::to_json() const {
	auto trace = tracer_impl_t::to_json();
	trace[name] = io::json::merge_objects(soft_failures, hard_failures, 
		total_power, total_dynamic_power, total_static_power,
		total_energy, total_dynamic_energy, total_static_energy,
		io::json(modules));
	return trace;
}

void time_tracer_t::trace(
	const working_set_t &ws, const insn_bits_t opc, const insn_t &insn) {
#if 0
	auto shared_timed_insn = hstd::shared_ptr<insn_info_t>(
		new insn_info_t(ws, opc, insn));
	
	core->buffer_insn(shared_timed_insn);
	if(hyperdrive_disabled) {
		core->update_pc(ws.pc);
		hyperdrive_disabled = false;
	}

	while(scheduler.ready() && !scheduler.empty() && 
		(core->get_clock() != TICK_LIMIT || !TICK_LIMIT_ENABLE)) {
		scheduler.tick();
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
#endif
}

double time_tracer_t::update_power_energy() {
	assert_msg(1 == 0, "Not implemented");
	return 0;
	#if 0
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
	#endif
}