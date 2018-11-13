#ifndef TIME_TRACER_H
#define TIME_TRACER_H

#include <string>
#include <map> 

#include "tracer.h"
#include "intermittent.h"
#include "event.h"

class component_base_t;
class core_t;
class time_tracer_t: public tracer_impl_t, public intermittent_t {
public:
	time_tracer_t(io::json _config, elfloader_t *_elf);
	~time_tracer_t();
	bool interested(working_set_t *ws, insn_bits_t opc, insn_t insn) {
		return true;
	}
	void trace(working_set_t *ws, insn_bits_t opc, insn_t insn);
	void reset(size_t minstret);
	void tabulate() {}
	io::json to_json() const;
private:
	event_list_t events;
	core_t *core = nullptr;
	std::map<std::string, component_base_t *> components;
};

#endif