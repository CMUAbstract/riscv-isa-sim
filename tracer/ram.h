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
	uint32_t read_latency;
	uint32_t write_latency;
	uint32_t ports;
	uint32_t read_ports;
	uint32_t write_ports;
	uint32_t bank_count;
	uint32_t load_buf_size;
	uint32_t store_buf_size;
	struct bank_info_t {
		uint32_t readers = 0;
		uint32_t readerq = 0;
		uint32_t writers = 0;
		uint32_t writerq = 0;
		uint32_t total() { return readers + writers; }
	};
	std::vector<bank_info_t> banks;
protected:
	addr_t get_bank(addr_t addr);
	uint32_t ports_per_bank;
	uint32_t read_ports_per_bank;
	uint32_t write_ports_per_bank;
	uint32_t bank_mask;
protected:
	counter_stat_t<uint64_t> reads;
	counter_stat_t<uint64_t> writes;
};

#endif