#ifndef CORE_H
#define CORE_H

#include <vector>

#include <common/decode.h>

#include "event.h"

class timed_insn_t;
class insn_fetch_event_t;
class insn_decode_event_t;
class insn_retire_event_t;
class reg_read_event_t;
class reg_write_event_t;
class mem_ready_event_t;
class mem_stall_event_t;
class core_t: public component_t {
public:
	core_t(std::string _name, io::json _config, event_list_t *_events);
	~core_t();
	virtual void buffer_insn(timed_insn_t *insn);
	virtual void process(insn_fetch_event_t *event) = 0;
	virtual void process(insn_decode_event_t *event) = 0;
	virtual void process(insn_retire_event_t *event) = 0;
	virtual void process(reg_read_event_t *event) = 0;
	virtual void process(reg_write_event_t *event) = 0;
	virtual void process(mem_ready_event_t *event) = 0;
	virtual void process(mem_stall_event_t *event) = 0;
protected:
	std::vector<timed_insn_t *> insns;
};

template<typename T>
core_t* create_core(std::string name, io::json config, event_list_t *events) {
	return new T(name, config, events);
}

#endif