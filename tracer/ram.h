#ifndef MEM_H
#define MEM_H

#include <fesvr/memif.h>
#include <stat/stat.h>

#include "component.h"
#include "signal_handler.h"

struct mem_read_event_t;
struct mem_write_event_t;
struct mem_insert_event_t;
class ram_t: public component_t, public signal_handler_t {
public:
	ram_t(std::string _name, io::json _config, event_list_t *_events);
	virtual ~ram_t() {}
	virtual void process(mem_read_event_t *event) = 0;
	virtual void process(mem_write_event_t *event)= 0;
	virtual void process(mem_insert_event_t *event) = 0;
	virtual io::json to_json() const;
protected:
	counter_stat_t<uint64_t> reads;
	counter_stat_t<uint64_t> writes;
};

#endif