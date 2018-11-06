#ifndef CACHE_H
#define CACHE_H

#include "ram.h"

#include <vector>

class repl_policy_t;
class mem_event_t;
class cache_t: public ram_t {
public:
	cache_t(std::string _name, io::json _config, event_list_t *_events);
	cache_t(std::string _name, uint32_t _lines, uint32_t _sets, event_list_t *_events)
		: ram_t(_name, nullptr, _events), lines(_lines), sets(_sets) { init(); }
	cache_t(std::string _name, uint32_t _lines, uint32_t _sets, 
		repl_policy_t *_repl_policy, event_list_t *_events)
		: ram_t(_name, nullptr, _events), lines(_lines), sets(_sets), 
		repl_policy(_repl_policy) { init(); }
	virtual ~cache_t() {}
	void set(repl_policy_t *_repl_policy) { repl_policy = _repl_policy; }
	void process(mem_read_event_t *event);
	void process(mem_write_event_t *event);
	void process(mem_insert_event_t *event);
	void process(ready_event_t *event);
	void process(stall_event_t *event);
protected:
	bool access(mem_event_t *event);
	void init();
	uint32_t read_latency = 1;
	uint32_t write_latency = 1;
	uint32_t invalid_latency = 1;
	uint32_t lines;
	uint32_t line_size = 4; 
	uint32_t sets;
	uint32_t offset_mask;
	uint32_t set_mask;
	uint32_t tag_mask;
	repl_policy_t *repl_policy = nullptr;
	std::vector<uint32_t> data; 
};

#endif