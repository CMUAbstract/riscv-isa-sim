#ifndef VECTOR_TRACER_H
#define VECTOR_TRACER_H

#include <stat/stat.h>

#include "tracer.h"

class mask_tracer_t : public tracer_impl_t {
public:
	mask_tracer_t(io::json _config, elfloader_t *_elf);
	~mask_tracer_t() { dump(); }
	bool interested(
		const working_set_t &ws, const insn_bits_t opc, const insn_t &insn);
	void trace(
		const working_set_t &ws, const insn_bits_t opc, const insn_t &insn);
	void tabulate() {}
	io::json to_json() const;
private:
	uint16_t vl;
	std::array<std::array<uint16_t, 0x10>, 0x10> vregs;
private:
	counter_stat_t<uint64_t> insn_count;
	counter_stat_t<uint64_t> mask_count;
	counter_stat_t<uint64_t> total_lanes;
	counter_stat_t<uint64_t> masked_lanes;
};

#endif