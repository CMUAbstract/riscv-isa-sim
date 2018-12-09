#ifndef MEM_H
#define MEM_H

#include <fesvr/memif.h>
#include <stat/stat.h>

#include "component.h"
#include "ram_handler.h"
#include "pending_handler.h"

class ram_t: public component_t<ram_t, ram_handler_t, pending_handler_t> {
public:
	ram_t(std::string _name, io::json _config, event_heap_t *_events);
	virtual ~ram_t() {}
	virtual void reset();
	virtual io::json to_json() const;
protected:
	counter_stat_t<uint64_t> reads;
	counter_stat_t<uint64_t> writes;
};

#endif