#ifndef VECTOR_HANDLER_H
#define VECTOR_HANDLER_H

struct vec_issue_event_t;
struct vec_start_event_t;
struct pe_exec_event_t;
struct pe_ready_event_t;
struct vec_reg_read_event_t;
struct vec_reg_write_event_t;
class vec_handler_t {
public:	
	virtual void process(vec_issue_event_t *) = 0;
	virtual void process(vec_start_event_t *) = 0;
	virtual void process(pe_exec_event_t *) = 0;
	virtual void process(pe_ready_event_t *) = 0;
	virtual void process(vec_reg_read_event_t *) = 0;
	virtual void process(vec_reg_write_event_t *) = 0;
};

struct vec_ready_event_t;
struct vec_retire_event_t;
class vec_signal_handler_t {
public:
	virtual void process(vec_ready_event_t *) = 0;
	virtual void process(vec_retire_event_t *) = 0;
};

#endif