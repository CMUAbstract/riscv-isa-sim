#ifndef MEM_H
#define MEM_H

#include <fesvr/memif.h>
#include <stat/stat.h>

#include "module.h"

class ram_t: public module_t {
public:
	ram_t(std::string _name, io::json _config, scheduler_t *_scheduler);
	virtual ~ram_t() {}
	virtual void reset(reset_level_t level);
	virtual io::json to_json() const;

	addr_t get_bank(addr_t addr);
	uint32_t get_line_size() { return line_size; }
protected:
	uint32_t line_size;
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
	uint32_t ports_per_bank;
	uint32_t read_ports_per_bank;
	uint32_t write_ports_per_bank;
	uint32_t bank_mask;
protected:
	counter_stat_t<uint32_t> bank_conflicts;
};

#endif