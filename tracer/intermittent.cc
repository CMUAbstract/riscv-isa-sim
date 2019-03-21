#include "intermittent.h"

#include <stdlib.h>
#include <sstream>
#include <fstream>

#include "log.h"

void intermittent_t::set_power_trace(std::string power_trace) {
	use_trace = true;
	std::ifstream in(power_trace, std::ios::in | std::ios::binary);

	if(!in.fail()) {
		std::ostringstream contents;
		double time = 0;
		double voltage = 0;
		while (in >> time >> voltage) {
			trace.time.push_back(time);
			trace.voltage.push_back(voltage);
			if(voltage > max_voltage) max_voltage = voltage;
			if(voltage < min_voltage) min_voltage =  voltage;
		}
		in.close();
	} else assert_msg(1 == 0, "Trace file does not exist");

	fprintf(stderr, "Done loading trace.\n");
	
	double vsquared = max_voltage * max_voltage - min_voltage * min_voltage;
	primary_energy = 0.5 * trace_info.primary.size * vsquared * 1e9; // Convert to nJ
	secondary_energy = 0.5 * trace_info.secondary.size * vsquared * 1e9;
	reserve_energy = 0.5 * trace_info.reserve.size * vsquared * 1e9;
	reset_should_fail();
}

void intermittent_t::calc_total_esr() {
	trace_info.total_esr = 1. / trace_info.primary.esr;
	if(trace_info.secondary.esr == 0) {
		trace_info.total_esr = trace_info.primary.esr; 
		return;
	}

	trace_info.total_esr += 1. / trace_info.secondary.esr;

	if(trace_info.reserve.esr == 0) return;
	trace_info.total_esr += 1. / trace_info.reserve.esr;
}

void intermittent_t::reset_should_fail() {
	fail_cycle = intermittent_min + (rand() % static_cast<int>(
        intermittent_max - intermittent_min + 1));
	charged_energy = 0.;
	soft_fail = false;
	recover = false;
}

double slope(const std::vector<double>& pts, size_t idx, size_t len) {
	double sumx = 0., sumx2 = 0., sumxy = 0., sumy = 0., sumy2 = 0.;

	for(size_t i = 0; i < len; i++) {
		sumx += i;
		sumx2 += i * i;
		sumxy += i * pts[idx + i];
		sumy += pts[idx + i];
		sumy2 += pts[idx + i] * pts[idx + i];
	}

	double denom = len * sumx2 - sumx * sumx;
	if(denom == 0) return 0;

	return (len * sumxy - sumx * sumy) / denom;
}

void intermittent_t::recharge_tick() {
	assert_msg(trace.idx < trace.time.size(), "Power trace exhausted");
	size_t smoothing = 10;
	trace.idx++;

	if(slope(trace.voltage, trace.idx, smoothing) < 0) {

#ifdef INTERMITTENT_LOG 
		fprintf(stderr, "Recovered; idx: %lu time: %f voltage: %f\n", trace.idx, 
			trace.time[trace.idx], trace.voltage[trace.idx]);
#endif

		trace.idx += smoothing;
		recover = true;
		while(slope(trace.voltage, trace.idx, smoothing) < 0) {
			trace.idx++;
		}
		trace.idx += smoothing;
	}
}

bool intermittent_t::should_fail(cycle_t cycle, double energy, uint32_t frequency) {
	if(!use_trace) return cycle >= fail_cycle;
	// Check for hard failure here
	if(soft_fail) return energy > primary_energy + secondary_energy;
	// Check for soft failure here
	soft_fail = energy > primary_energy;
	return soft_fail;
}