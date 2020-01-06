#ifndef MEM_TRACER_H
#define MEM_TRACER_H

#include "tracer.h"

#include <stat/stat.h>

class mem_tracer_t : public tracer_impl_t {
private:
	class access_stat_t : public stat_t {
	public:
		access_stat_t() : access_stat_t("", "") {}
		access_stat_t(std::string _name, std::string _desc) {}
		io::json to_json() const;
	public:
		enum {VRFR, VRFW, VLOAD, VSTORE, LOAD, STORE} type;
		addr_t addr;
		addr_t pc;
		uint8_t width;
		uint32_t update;
	};
public:
	mem_tracer_t(io::json _config, elfloader_t *_elf)
		: tracer_impl_t("mem_tracer", _config, _elf), transforms("transforms") {}
	~mem_tracer_t();
	bool interested(const working_set_t &ws, const insn_bits_t opc, const insn_t &insn);
	void trace(const working_set_t &ws, const insn_bits_t opc, const insn_t &insn);
	void tabulate() {}
	io::json to_json() const { return transforms; }	
private:
	void account_vrf(const working_set_t &ws, std::vector<mem_tracer_t::access_stat_t*>& accesses);
private:
	uint32_t vl = 0;
	cycle_t cycle = 0;
	addr_t last_pc = 0;
	map_stat_t<cycle_t, std::vector<access_stat_t*>> transforms;	
};

#endif