#ifndef CORE_H
#define CORE_H

#include <vector>

#include <common/decode.h>
#include <stat/stat.h>
#include <hstd/memory.h>

#include "working_set.h"
#include "component.h"
#include "signal_handler.h"
#include "pending_handler.h"
#include "squash_handler.h"
#include "vector_handler.h"

struct timed_insn_t {
	timed_insn_t(working_set_t _ws, insn_bits_t _opc, insn_t _insn)
		: ws(_ws), opc(_opc), insn(_insn) {}
	working_set_t ws;
	insn_bits_t opc;
	insn_t insn;
};

struct insn_fetch_event_t;
struct insn_decode_event_t;
struct insn_exec_event_t;
struct insn_retire_event_t;
struct reg_read_event_t;
struct reg_write_event_t;

class core_handler_t {
public:
	virtual void process(insn_fetch_event_t *event) = 0;
	virtual void process(insn_decode_event_t *event) = 0;
	virtual void process(insn_exec_event_t *event) = 0;
	virtual void process(insn_retire_event_t *event) = 0;
	virtual void process(reg_read_event_t *event) = 0;
	virtual void process(reg_write_event_t *event) = 0;
};

class core_t: public component_t<core_t, signal_handler_t, core_handler_t, 
	pending_handler_t, squash_handler_t> {
public:
	core_t(std::string _name, io::json _config, event_heap_t *_events);
	~core_t() {}
	virtual void reset();
	virtual io::json to_json() const;
	virtual void buffer_insn(hstd::shared_ptr<timed_insn_t> insn) = 0;
	virtual size_t minstret() const { return retired_insns.get(); }
protected:
	std::vector<hstd::shared_ptr<timed_insn_t>> insns;
	size_t insn_idx = 0;
	reg_t pc = 0x1000;
	std::map<std::string, bool> state;
protected: // stats
	counter_stat_t<size_t> retired_insns;
	counter_stat_t<size_t> running_insns;
};

#endif