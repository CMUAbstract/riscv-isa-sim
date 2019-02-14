#ifndef RAM_HANDLER_H
#define RAM_HANDLER_H

struct event_base_t;
struct mem_read_event_t;
struct mem_write_event_t;
struct mem_insert_event_t;
class ram_handler_t {
public:		
	virtual void process(mem_read_event_t *) = 0;
	virtual void process(mem_write_event_t *) = 0;
	virtual void process(mem_insert_event_t *) = 0;
};

struct mem_ready_event_t;
struct mem_retire_event_t;
struct mem_match_event_t;
class ram_signal_handler_t {
public:
	virtual void process(mem_ready_event_t *) = 0;
	virtual void process(mem_retire_event_t *) = 0;
};

#endif