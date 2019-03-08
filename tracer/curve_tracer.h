#ifndef CURVE_TRACER_H
#define CURVE_TRACER_H

#include <list>
#include <map>

#include <stat/stat.h>

#include "tracer.h"

// Generate Miss curves
class insn_curve_tracer_t : public tracer_impl_t {
public:
	insn_curve_tracer_t(io::json _config, elfloader_t *_elf);
	~insn_curve_tracer_t();
	bool interested(
		const working_set_t &ws, const insn_bits_t opc, const insn_t &insn) {
		return true;
	}
	void trace(
		const working_set_t &ws, const insn_bits_t opc, const insn_t &insn);
	void tabulate() {}
	io::json to_json() const;
protected:
	std::map<addr_t, reg_t> tracked_locations;
	bool text_calc = false;
	addr_t text_base = 0;
	uint32_t text_size = 0;
	bool serialized = false;
protected: // Stats
	map_stat_t<uint32_t, counter_stat_t<uint32_t> *> histogram;
	counter_stat_t<uint64_t> minstret;
};

class miss_curve_tracer_t : public insn_curve_tracer_t {
public:
	miss_curve_tracer_t(io::json _config, elfloader_t *_elf);
	~miss_curve_tracer_t();
	void trace(
		const working_set_t &ws, const insn_bits_t opc, const insn_t &insn);
	io::json to_json() const;
private:
	bool lru = false;
	bool exclude_text = true;
private:
	std::list<addr_t> dq;
	std::map<addr_t, std::list<addr_t>::iterator> indices;
private:
	uint32_t max_dist = 0;
	uint32_t line_size = 4;
	uint32_t line_mask = 0;
	counter_stat_t<uint64_t> maccess;
	counter_stat_t<uint64_t> cold_misses;
};

#endif