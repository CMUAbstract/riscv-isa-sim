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
	bool interested(
		const working_set_t &ws, const insn_bits_t opc, const insn_t &insn) {
		return true;
	}
	void trace(
		const working_set_t &ws, const insn_bits_t opc, const insn_t &insn);
	void reset(reset_level_t level, uint32_t minstret);
	void tabulate();
	io::json to_json() const;
	void set_hyperdrive(bool _hyperdrive=true) {
		if(!_hyperdrive && hyperdrive) hyperdrive_disabled = true;
		hyperdrive = _hyperdrive;
	}
	void set_intermittent(bool _intermittent=true) {
		this->intermittent = _intermittent;
	} 
private:
	event_heap_t events;
	core_t *core = nullptr;
	bool hyperdrive_disabled = false;
	std::map<std::string, component_base_t *> components;
	counter_stat_t<uint32_t> soft_failures;
	counter_stat_t<uint32_t> hard_failures;
};

#endif