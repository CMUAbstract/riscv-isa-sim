#ifndef CORE_H
#define CORE_H

#include <vector>

#include <common/decode.h>

#include "event.h"
#include "mem.h"

class working_set_t;
struct timed_insn_t {
	timed_insn_t(working_set_t *_ws, insn_bits_t _opc, insn_t _insn)
		: ws(_ws), opc(_opc), insn(_insn) {}
	working_set_t *ws;
	insn_bits_t opc;
	insn_t insn;
};

template <typename T>
struct insn_fetch_event_t : public event_t<T> {
	insn_fetch_event_t(T *_handler, working_set_t *_ws, insn_bits_t _opc, 
		insn_t _insn) : event_t<T>(_handler), insn(_ws, _opc, _insn) {}
	insn_fetch_event_t(working_set_t *_ws, insn_bits_t _opc, insn_t _insn) 
		: insn(_ws, _opc, _insn) {}
	insn_fetch_event_t(T *_handler, timed_insn_t *_insn) 
		: event_t<T>(_handler), insn(_insn->ws, _insn->opc, _insn->insn) {}
	insn_fetch_event_t(timed_insn_t *_insn) 
		: insn(_insn->ws, _insn->opc, _insn->insn) {}
	HANDLER;
	timed_insn_t insn;	
};

class core_t: public component_t {
public:
	core_t(io::json _config, event_list_t *_events, mem_t *_mm);
	~core_t();
	virtual void buffer_insn(timed_insn_t *insn);
	virtual void process(insn_fetch_event_t<core_t> *event) = 0;
	virtual void process(reg_read_event_t<core_t> *event) = 0;
	virtual void process(reg_write_event_t<core_t> *event) = 0;
protected:
	mem_t *mm;
	std::vector<timed_insn_t *> insns;
};

template<typename T> core_t* create_core(
	io::json config, event_list_t *events, mem_t *mem) {
	return new T(config, events, mem);
}

#endif