#ifndef MEM_EVENT_H
#define MEM_EVENT_H

#include "event.h"
#include "mem.h"
#include "core.h"

struct reg_read_event_t: public event_t<core_t> {
	reg_read_event_t(core_t *_handler, size_t _reg, cycle_t _cycle)
		: event_t<core_t>(_handler, _cycle, 1) {}
	reg_read_event_t(size_t _reg, cycle_t _cycle)
		: reg_read_event_t(nullptr, _reg, _cycle) {}
	HANDLER;
	size_t reg;
};

struct reg_write_event_t: public event_t<core_t> {
	reg_write_event_t(core_t *_handler, size_t _reg, cycle_t _cycle)
		: event_t<core_t>(_handler, _cycle, 1) {}
	reg_write_event_t(size_t _reg, cycle_t _cycle)
		: reg_write_event_t(nullptr, _reg, _cycle) {}
	HANDLER;
	size_t reg;
};

struct mem_event_t: public event_t<mem_t> {
	mem_event_t(mem_t *_handler, addr_t _addr, cycle_t _cycle) 
		: event_t<mem_t>(_handler, _cycle), addr(_addr) {}
	mem_event_t(addr_t _addr, cycle_t _cycle)
		: mem_event_t(nullptr, _addr, _cycle) {}
	addr_t addr;
};

struct mem_read_event_t: public mem_event_t {
	using mem_event_t::mem_event_t;
	HANDLER;
};

struct mem_write_event_t: public mem_event_t {
	using mem_event_t::mem_event_t;
	HANDLER;
};

struct mem_insert_event_t: public mem_event_t {
	using mem_event_t::mem_event_t;
	HANDLER;
};

#endif