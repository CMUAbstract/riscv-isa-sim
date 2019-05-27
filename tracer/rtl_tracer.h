#ifndef RTL_TRACER_H
#define RTL_TRACER_H

#include "tracer.h"

#include <stat/stat.h>

class rtl_tracer_t: public tracer_impl_t {
public:
	rtl_tracer_t(io::json _config, elfloader_t *_elf);
	~rtl_tracer_t() {}
	bool interested(
		const working_set_t &ws, const insn_bits_t opc, const insn_t &insn) {
		return true;
	}
	void trace(
		const working_set_t &ws, const insn_bits_t opc, const insn_t &insn);
	void tabulate() {}
	io::json to_json() const;
private:
	map_stat_t<uint32_t, reg_t> registers;
	map_stat_t<addr_t, reg_t> locations;
	addr_t start_addr;
	addr_t end_addr;
	bool loaded = false;
	bool tracking = false;
};

#endif