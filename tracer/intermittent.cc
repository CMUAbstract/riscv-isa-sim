#include "intermittent.h"

#include <stdlib.h>
#include <sstream>
#include <fstream>

#include "log.h"

void intermittent_t::set_power_trace(std::string power_trace) {
	use_trace = true;
	std::ifstream in(power_trace, std::ios::in | std::ios::binary);
	if(in) {
		std::ostringstream contents;
		uint64_t time = 0;
		double voltage = 0;
		uint64_t total_time = 0;
		while (in >> time >> voltage) {
			trace.time.push_back(total_time);
			trace.voltage.push_back(voltage);
			total_time += time;
		}
    	in.close();
	}
}

void intermittent_t::calc_total_esr() {
	trace_info.total_esr = 1. / trace_info.primary.esr;
	trace_info.total_esr += 1. / trace_info.secondary.esr;
	trace_info.total_esr += 1. / trace_info.reserve.esr;
}

void intermittent_t::reset_should_fail() {
	fail_cycle = intermittent_min + (rand() % static_cast<int>(
        intermittent_max - intermittent_min + 1));
	double vsquared = avg_voltage * avg_voltage;
	primary_energy = 0.5 * trace_info.primary.size * vsquared;
	secondary_energy = 0.5 * trace_info.secondary.size * vsquared;
	reserve_energy = 0.5 * trace_info.reserve.size * vsquared;
	charged_energy = 0.;
	soft_fail = false;
	recover = false;
}

void intermittent_t::recharge_tick(uint32_t frequency) {
	assert_msg(trace.idx < trace.time.size(), "Power trace exhausted");
	double deltaT = (trace.time[trace.idx] - trace.time[trace.idx + 1]) * 1e-3;
	double charge = trace.voltage[trace.idx] * trace.voltage[trace.idx];
	charged_energy += (charge / trace_info.total_esr) * deltaT;
	trace.idx++;

	if(charged_energy > primary_energy + secondary_energy + reserve_energy) 
		recover = true;
}

bool intermittent_t::should_fail(cycle_t cycle, double energy, uint32_t frequency) {
	if(!use_trace) return cycle >= fail_cycle;
	// Check for hard failure here
	if(soft_fail) return energy > primary_energy + secondary_energy;
	// Check for soft failure here
	soft_fail = energy > primary_energy;
	return soft_fail;
}