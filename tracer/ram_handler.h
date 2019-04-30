#ifndef RAM_HANDLER_H
#define RAM_HANDLER_H

struct value_base_t;
struct mem_read_value_t;
struct mem_write_value_t;
struct mem_insert_value_t;
class ram_handler_t {
public:		
	virtual void process(mem_read_value_t) = 0;
	virtual void process(mem_write_value_t) = 0;
	virtual void process(mem_insert_value_t) = 0;
};

struct mem_ready_value_t;
struct mem_retire_value_t;
struct mem_match_value_t;
class ram_signal_handler_t {
public:
	virtual void process(mem_ready_value_t) = 0;
	virtual void process(mem_retire_value_t) = 0;
};

#endif