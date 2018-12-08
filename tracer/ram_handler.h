#ifndef RAM_HANDLER_H
#define RAM_HANDLER_H

#include "handler.h"

struct mem_read_event_t;
struct mem_write_event_t;
struct mem_insert_event_t;
class ram_handler_t: 
	public handler_t<mem_read_event_t *>, handler_t<mem_write_event_t *>,
	public handler_t<mem_insert_event_t *> {
public:		
	using handler_t<mem_read_event_t *>::process;
	using handler_t<mem_write_event_t *>::process;
	using handler_t<mem_insert_event_t *>::process;
};

struct mem_ready_event_t;
struct mem_match_event_t;
class ram_signal_handler_t: 
	public handler_t<mem_ready_event_t *>, handler_t<mem_match_event_t *> {
public:
	using handler_t<mem_ready_event_t *>::process;
	using handler_t<mem_match_event_t *>::process;
};

#endif