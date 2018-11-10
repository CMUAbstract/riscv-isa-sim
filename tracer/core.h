#ifndef CORE_H
#define CORE_H

#include <vector>

#include <common/decode.h>
#include <stat/stat.h>

#include "component.h"
#include "signal_handler.h"

struct timed_insn_t;
struct insn_fetch_event_t;
struct insn_decode_event_t;
struct insn_retire_event_t;
struct reg_read_event_t;
struct reg_write_event_t;
class core_t: public component_t, public signal_handler_t {
public:
	core_t(std::string _name, io::json _config, event_list_t *_events);
	~core_t();
	virtual void reset();
	virtual io::json to_json() const;
	virtual void buffer_insn(timed_insn_t *insn);
	virtual void process(insn_fetch_event_t *event) = 0;
	virtual void process(insn_decode_event_t *event) = 0;
	virtual void process(insn_retire_event_t *event) = 0;
	virtual void process(reg_read_event_t *event) = 0;
	virtual void process(reg_write_event_t *event) = 0;
	virtual size_t minstret() const { return retired_insns.get(); }
protected:
	std::vector<timed_insn_t *> insns;
	// stats
	counter_stat_t<size_t> retired_insns;
	counter_stat_t<size_t> total_insns;
};

#endif