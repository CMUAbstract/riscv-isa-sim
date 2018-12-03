#ifndef VECTOR_HANDLER_H
#define VECTOR_HANDLER_H

struct vector_exec_event_t;
struct pe_exec_event_t;
struct pe_ready_event_t;
class vector_handler_t {
public:
	virtual void process(vector_exec_event_t *event) = 0;
	virtual void process(pe_exec_event_t *event) = 0;
	virtual void process(pe_ready_event_t *event) = 0;
};

struct vector_ready_event_t;
class vector_signal_handler_t {
public:
	virtual void process(vector_ready_event_t *event) = 0;
};

#endif