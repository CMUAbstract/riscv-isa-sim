#ifndef CORE_H
#define CORE_H

#include <vector>

#include <common/decode.h>

#include "event.h"
#include "mem.h"

class timed_insn_t;
class insn_fetch_event_t;
class insn_decode_event_t;
class reg_read_event_t;
class reg_write_event_t;
class core_t: public component_t {
public:
	core_t(io::json _config, event_list_t *_events, mem_t *_mm);
	~core_t();
	virtual void buffer_insn(timed_insn_t *insn);
	virtual void process(insn_fetch_event_t *event) = 0;
	virtual void process(insn_decode_event_t *event) = 0;
	virtual void process(reg_read_event_t *event) = 0;
	virtual void process(reg_write_event_t *event) = 0;
protected:
	mem_t *mm;
	std::vector<timed_insn_t *> insns;
};

template<typename T> core_t* create_core(
	io::json config, event_list_t *events, mem_t *mem) {
	return new T(config, events, mem);
}

#endif