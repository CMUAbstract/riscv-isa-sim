#ifndef VECTOR_HANDLER_H
#define VECTOR_HANDLER_H

#include "handler.h"

struct vector_exec_event_t;
struct pe_exec_event_t;
struct pe_ready_event_t;
struct vector_reg_read_event_t;
struct vector_reg_write_event_t;
class vector_handler_t: 
	public handler_t<vector_exec_event_t *>, public handler_t<pe_exec_event_t *>,
	public handler_t<pe_ready_event_t *>, public handler_t<vector_reg_read_event_t *>,
	public handler_t<vector_reg_write_event_t *> {
public:	
	using handler_t<vector_exec_event_t *>::process;
	using handler_t<pe_exec_event_t *>::process;
	using handler_t<pe_ready_event_t *>::process;
	using handler_t<vector_reg_read_event_t *>::process;
	using handler_t<vector_reg_write_event_t *>::process;
};

struct vector_ready_event_t;
struct vector_retire_event_t;
class vector_signal_handler_t:
	public handler_t<vector_ready_event_t *>, public handler_t<vector_retire_event_t *> {
public:
	using handler_t<vector_ready_event_t *>::process;
	using handler_t<vector_retire_event_t *>::process;
};

#endif