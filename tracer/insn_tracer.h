#ifndef INSN_TRACER_H
#define INSN_TRACER_H

#include "tracer.h"

#include <stat/stat.h>

class insn_tracer_t : public tracer_impl_t {
public:
	insn_tracer_t(io::json _config, elfloader_t *_elf)
		: tracer_impl_t(_config, _elf) {
		if(!insn_registered) register_insn_types();
		insn_registered = true;
	}
	void tabulate() {};
	~insn_tracer_t() {}
protected:
	static std::map<insn_bits_t, uint64_t> m;
private:
	void register_insn_types(void);
	static bool insn_registered;
};

class perf_tracer_t : public insn_tracer_t {
public:
	perf_tracer_t(io::json _config, elfloader_t *_elf)
		: insn_tracer_t(_config, _elf), mcycles("cycles", "") {
			mcycles.reset();
	}
	~perf_tracer_t() {}
	bool interested(working_set_t *ws, insn_bits_t opc, insn_t insn) {
		return true;
	}
	void trace(working_set_t *ws, insn_bits_t opc, insn_t insn);
	std::string dump();
private:
	counter_stat_t<uint64_t> mcycles;
};

class energy_tracer_t : public insn_tracer_t {
public:
	energy_tracer_t(io::json _config, elfloader_t *_elf)
		: insn_tracer_t(_config, _elf), menergy("energy", "") {
		menergy.reset();
	}
	~energy_tracer_t() {}
	bool interested(working_set_t *ws, insn_bits_t opc, insn_t insn) {
		return true;
	}
	void trace(working_set_t *ws, insn_bits_t opc, insn_t insn);
	std::string dump();
private:
	counter_stat_t<double> menergy; 
};

#endif