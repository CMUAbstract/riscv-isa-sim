#ifndef INTERMITTENT_H
#define INTERMITTENT_H

#include <vector>
#include <string>

#include <common/decode.h>

// #define INTERMITTENT_LOG 1

class intermittent_t {
protected:
	intermittent_t() { reset_should_fail(); }
	void set_power_trace(std::string power_trace);
	void reset_should_fail();
	void recharge_tick(uint32_t frequency);
	bool should_fail(cycle_t cycle, double energy, uint32_t frequency);
	bool recovered() { return recover; }
	void calc_total_esr();
protected:
	bool intermittent = false;
	uint32_t intermittent_max = 350000;
	uint32_t intermittent_min = 150000;
	typedef struct {
		double size = 1e-3;
		double esr = 1e-3;
	} cap_info_t;
	struct {
		cap_info_t primary;
		cap_info_t secondary;
		cap_info_t reserve;
		double total_esr = 1;
	} trace_info;
protected:
	double primary_energy = 0.;
	double secondary_energy = 0.;
	double reserve_energy = 0.;
	double charged_energy = 0.;
private:
	bool use_trace = false;
	bool soft_fail = false;
	bool recover = false;
	struct {
		std::vector<uint64_t> time;
		std::vector<double> voltage;
		uint64_t idx = 0;
	} trace;
	cycle_t fail_cycle = 0;
	double avg_voltage = 1.0;
};

#endif