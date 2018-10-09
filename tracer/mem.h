#ifndef MEM_H
#define MEM_H

#include <fesvr/memif.h>

#include "event.h"

template <typename T>
struct reg_read_event_t: public event_t<T> {
	reg_read_event_t(T *_handler, size_t _reg, cycle_t _cycle)
		: event_t<T>(_handler) {
		this->cycle = _cycle;
		this->latency = 1;
	}
	reg_read_event_t(size_t _reg, cycle_t _cycle)
		: reg_read_event_t(nullptr, _reg, _cycle) {}
	HANDLER;
	size_t reg;
};

template <typename T>
struct reg_write_event_t: public event_t<T> {
	reg_write_event_t(T *_handler, size_t _reg, cycle_t _cycle)
		: event_t<T>(_handler) {
		this->cycle = _cycle;
		this->latency = 1;
	}
	reg_write_event_t(size_t _reg, cycle_t _cycle)
		: reg_write_event_t(nullptr, _reg, _cycle) {}
	HANDLER;
	size_t reg;
};

template <typename T>
struct mem_read_event_t: public event_t<T> {
	mem_read_event_t(T *_handler, addr_t _addr, cycle_t _cycle) 
		: event_t<T>(_handler), addr(_addr) {
		this->cycle = _cycle;
	}
	mem_read_event_t(addr_t _addr, cycle_t _cycle)
		: mem_read_event_t(nullptr, _addr, _cycle) {}
	HANDLER;
	addr_t addr;
};

template <typename T>
struct mem_write_event_t: public event_t<T> {
	mem_write_event_t(T *_handler, addr_t _addr, cycle_t _cycle) 
		: event_t<T>(_handler), addr(_addr) {
		this->cycle = _cycle;
	}
	mem_write_event_t(addr_t _addr, cycle_t _cycle)
		: mem_write_event_t(nullptr, _addr, _cycle) {}
	HANDLER;
	addr_t addr;
};

template <typename T>
struct mem_miss_event_t: public event_t<T> {
	mem_miss_event_t(T *_handler) : event_t<T>(_handler) {}
	mem_miss_event_t() {}
	HANDLER;
};

template <typename T>
struct mem_invalidate_event_t: public event_t<T> {
	mem_invalidate_event_t(T *_handler) : event_t<T>(_handler) {}
	mem_invalidate_event_t() {}
	HANDLER;
};

class mem_t: public component_t {
public:
	using component_t::component_t;
	virtual void process(mem_read_event_t<mem_t> *event) = 0;
	virtual void process(mem_write_event_t<mem_t> *event) = 0;
	virtual void process(mem_miss_event_t<mem_t> *event) = 0;
	virtual void process(mem_invalidate_event_t<mem_t> *event) = 0;
};

template<typename T> mem_t* create_mem(io::json config, event_list_t *events) {
	return new T(config, events);
}

#endif