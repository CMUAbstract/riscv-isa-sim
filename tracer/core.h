#ifndef CORE_H
#define CORE_H

#include <deque>

#include <common/decode.h>
#include <stat/stat.h>
#include <hstd/memory.h>

#include "working_set.h"
#include "component.h"
#include "ram.h"
#include "core_handler.h"
#include "pending_handler.h"
#include "squash_handler.h"
#include "vector_handler.h"

struct timed_insn_t {
	timed_insn_t(working_set_t _ws, insn_bits_t _opc, insn_t _insn)
		: ws(_ws), opc(_opc), insn(_insn) {}
	working_set_t ws;
	insn_bits_t opc;
	insn_t insn;
	bool resolved = false;
	uint32_t idx = 0;
};

class core_t: public component_t<core_t, core_handler_t, 
	pending_handler_t, squash_handler_t, ram_signal_handler_t> {
public:
	core_t(std::string _name, io::json _config, event_heap_t *_events);
	~core_t() {}
	virtual void reset();
	virtual io::json to_json() const;
	virtual void buffer_insn(hstd::shared_ptr<timed_insn_t> insn) = 0;
	virtual void next_insn() = 0;
	virtual uint32_t minstret() const { return retired_insns.get(); }
	virtual void update_pc(reg_t _pc) { pc = _pc; }
protected:
	std::deque<hstd::shared_ptr<timed_insn_t>> insns;
	uint32_t insn_idx = 0;
	uint32_t retired_idx = 0;
	reg_t pc = 0x1000;
	std::map<std::string, bool> stages;
	bool check_jump(insn_bits_t opc);
protected: // stats
	counter_stat_t<uint32_t> retired_insns;
	counter_stat_t<uint32_t> running_insns;
};

#endif