#ifndef CACHE_H
#define CACHE_H

#include <vector>

#include <stat/stat.h>

#include "ram.h"

class repl_policy_t;
class mem_event_t;
class cache_t: public ram_t {
public:
	cache_t(std::string _name, io::json _config, event_heap_t *_events);
	virtual ~cache_t() {}
	virtual void reset();
	virtual io::json to_json() const;	
	void set(repl_policy_t *_repl_policy) { repl_policy = _repl_policy; }
	void process(mem_read_event_t *event);
	void process(mem_write_event_t *event);
	void process(mem_insert_event_t *event);
	void process(pending_event_t *event);
	void process(mem_ready_event_t *event) {}
	void process(mem_match_event_t *event) {}
protected:
	bool access(mem_event_t *event);
	uint32_t get_tag(addr_t addr);
	uint32_t get_set(addr_t addr);
	uint32_t read_latency = 1;
	uint32_t write_latency = 1;
	uint32_t invalid_latency = 1;
	uint32_t ports = 1;
	uint32_t lines;
	uint32_t line_size = 4; 
	uint32_t sets;
	uint32_t offset_mask;
	uint32_t set_mask;
	uint32_t set_offset = 0;
	uint32_t set_size;
	uint32_t tag_mask;
	repl_policy_t *repl_policy;
	std::vector<uint32_t> data; 
protected:
	// Stats
	counter_stat_t<uint64_t> accesses;	
	counter_stat_t<uint64_t> inserts;
	counter_stat_t<uint64_t> read_misses;	
	counter_stat_t<uint64_t> write_misses;	
	counter_stat_t<uint64_t> read_hits;	
	counter_stat_t<uint64_t> write_hits;	
};

#endif