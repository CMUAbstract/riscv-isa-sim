#ifndef BASIC_TRACER_H
#define BASIC_TRACER_H

#include "tracer.h"

#include <stat/stat.h>

// Tracks reads and writes to locations in memory
class basic_ram_tracer_t : public tracer_impl_t {
public:
	basic_ram_tracer_t(io::json _config, elfloader_t *_elf)
		: tracer_impl_t(_config, _elf) {}
	~basic_ram_tracer_t();
	bool interested(working_set_t *ws, insn_bits_t opc, insn_t insn) {
		return true;
	}
	void trace(working_set_t *ws, insn_bits_t opc, insn_t insn);
	void tabulate();
	std::string dump();
private:
	class mem_loc_stat_t : public stat_t {
	public:
		mem_loc_stat_t() : mem_loc_stat_t("", "") {}
		mem_loc_stat_t(std::string _name, std::string _desc) :
			stat_t(_name, _desc), reads("reads", ""), writes("writes", "") {
			reads.reset();	
			writes.reset();
		}
		std::string symbol;
		std::string section;
		counter_stat_t<size_t> reads;
		counter_stat_t<size_t> writes;
		io::json to_json() const {
			return io::json::object{
				{"symbol", symbol},
				{"section", section},
				{"reads", reads},
				{"writes", writes}
			};
		}
	};
	map_stat_t<addr_t, mem_loc_stat_t *> tracked_locations;
private:
	struct region_t {
		addr_t base;
		size_t size;
		region_t() : region_t(0, 0) {}
		region_t(addr_t _base, size_t _size) : base(_base), size(_size) {}
		region_t(const region_t &other) {base = other.base; size = other.size; } 
	};
};

#endif