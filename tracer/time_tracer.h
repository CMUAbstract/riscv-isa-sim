#ifndef TIME_TRACER_H
#define TIME_TRACER_H

#include <vector>

#include <stat/stat.h>

#include "tracer.h"
#include "event.h"

class core_t;
class mem_t;
class time_tracer_t: public tracer_impl_t {
public:
	time_tracer_t(io::json _config, elfloader_t *_elf);
	~time_tracer_t();
	bool interested(working_set_t *ws, insn_bits_t opc, insn_t insn) {
		return true;
	}
	void trace(working_set_t *ws, insn_bits_t opc, insn_t insn);
	void tabulate() {}
	std::string dump();
private:
	event_list_t events;
	counter_stat_t<cycle_t> cycle;
	core_t *core = nullptr;
	mem_t *mem = nullptr;
};

#endif