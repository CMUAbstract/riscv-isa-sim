#ifndef MEM_EVENT_H
#define MEM_EVENT_H

#include "event.h"
#include "core.h"
#include "mem.h"

struct reg_read_event_t: public event_t<core_t, reg_t> {
	using event_t<core_t, reg_t>::event_t;
	HANDLER;
};

struct reg_write_event_t: public event_t<core_t, reg_t> {
	using event_t<core_t, reg_t>::event_t;
	HANDLER;
};

struct mem_event_t: public event_t<mem_t, addr_t> {
	using event_t<mem_t, addr_t>::event_t;
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

struct mem_ready_event_t: public stall_event_t<addr_t> {
	using stall_event_t<addr_t>::stall_event_t;
	HANDLER;
};

struct mem_stall_event_t: public ready_event_t<addr_t> {
	using ready_event_t<addr_t>::ready_event_t;
	HANDLER;
};

#endif