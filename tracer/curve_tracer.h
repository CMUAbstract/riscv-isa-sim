#ifndef CURVE_TRACER_H
#define CURVE_TRACER_H

#include <map>

#include <stat/stat.h>

#include "tracer.h"

// Generate Miss curves
class insn_curve_tracer_t : public tracer_impl_t {
public:
	insn_curve_tracer_t(io::json _config, elfloader_t *_elf)
		: tracer_impl_t("insn_curve_tracer", _config, _elf) {
		init();
	}
	~insn_curve_tracer_t();
	bool interested(
		const working_set_t &ws, const insn_bits_t opc, const insn_t &insn) {
		return true;
	}
	void trace(
		const working_set_t &ws, const insn_bits_t opc, const insn_t &insn);
	void tabulate() {}
	io::json to_json() const { return io::json(histogram); }
protected:
	void init(void);
	std::map<addr_t, reg_t> tracked_locations;
	map_stat_t<size_t, counter_stat_t<size_t> *> histogram;
	addr_t text_base = 0;
	size_t text_size = 0;
private:
	reg_t minstret = 0;
};

class miss_curve_tracer_t : public insn_curve_tracer_t {
public:
	miss_curve_tracer_t(io::json _config, elfloader_t *_elf) 
		: insn_curve_tracer_t(_config, _elf) {
		name = "miss_curve_tracer"; 
	}
	void trace(
		const working_set_t &ws, const insn_bits_t opc, const insn_t &insn);
private:
	reg_t maccess = 0;
};

#endif