#ifndef CORE_H
#define CORE_H

#include <vector>

#include <common/decode.h>

#include "component.h"
#include "signal.h"

struct timed_insn_t;
struct insn_fetch_event_t;
struct insn_decode_event_t;
struct insn_retire_event_t;
struct reg_read_event_t;
struct reg_write_event_t;
class core_t: public component_t, public signal_handler_t, virtual event_handler_t {
public:
	core_t(std::string _name, io::json _config, event_list_t *_events);
	~core_t();
	virtual void buffer_insn(timed_insn_t *insn);
	virtual void process(insn_fetch_event_t *event) = 0;
	virtual void process(insn_decode_event_t *event) = 0;
	virtual void process(insn_retire_event_t *event) = 0;
	virtual void process(reg_read_event_t *event) = 0;
	virtual void process(reg_write_event_t *event) = 0;
	virtual void process(ready_event_t *event) = 0;
	virtual void process(stall_event_t *event) = 0;
protected:
	std::vector<timed_insn_t *> insns;
};

template<typename T>
core_t* create_core(std::string name, io::json config, event_list_t *events) {
	return new T(name, config, events);
}

#endif