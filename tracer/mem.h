#ifndef MEM_H
#define MEM_H

#include <fesvr/memif.h>

#include "event.h"

struct reg_read_event_t: public event_t {
	reg_read_event_t(size_t _reg, cycle_t _cycle) : reg(_reg) {
		cycle = _cycle;
		latency = 1;
	}
	size_t reg;
};

struct reg_write_event_t: public event_t {
	reg_write_event_t(size_t _reg, cycle_t _cycle) : reg(_reg) {
		cycle = _cycle;
		latency = 1;
	}
	size_t reg;
};

struct mem_read_event_t: public event_t {
	mem_read_event_t(addr_t _addr, cycle_t _cycle) : addr(_addr) {
		cycle = _cycle;
	}
	addr_t addr;
};

struct mem_write_event_t: public event_t {
	mem_write_event_t(addr_t _addr, cycle_t _cycle) : addr(_addr) {
		cycle = _cycle;
	}
	addr_t addr;
};

struct mem_miss_event_t: public event_t {
	mem_miss_event_t() {}
};

struct mem_invalidate_event_t: public event_t {
	mem_invalidate_event_t() {}
};

class mem_t: public component_t {
	using component_t::component_t;
	void process(event_t event) {}
	virtual void process(mem_read_event_t event) = 0;
	virtual void process(mem_write_event_t event) = 0;
	virtual void process(mem_miss_event_t event) = 0;
	virtual void process(mem_invalidate_event_t event) = 0;
};

#endif