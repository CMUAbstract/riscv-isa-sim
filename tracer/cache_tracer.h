#ifndef CACHE_TRACER_H
#define CACHE_TRACER_H

#include <map>
#include <tuple>

#include <stat/stat.h>

#include "tracer.h"

class cache_tracer_t : public tracer_impl_t {
protected:
	class cache_t {
	public: 
		cache_t() {};
		void init(uint32_t _block_size, uint32_t _ways, uint32_t _size);
		std::tuple<bool, bool, bool> update(addr_t addr, bool write=false);
	private:
		uint32_t get_set(uint32_t addr) {
			return (addr >> set_offset) & set_mask;
		}
		uint32_t get_tag(uint32_t addr) {
			return (addr >> tag_offset) & tag_mask;
		}
	private:
		uint32_t sets = 128;
		uint32_t set_mask = (1 << 7) - 1;
		uint32_t set_offset = 2 + 2;
		uint32_t tag_mask = 0;
		uint32_t tag_offset = 0;
		uint32_t block_size = 4;
		uint32_t ways = 2;
		uint32_t size = 256;
		std::vector<std::vector<addr_t>> tag;
		std::vector<std::vector<uint32_t>> lru;
		std::vector<std::vector<bool>> valid;
		std::vector<std::vector<bool>> dirty;
	};
public:
	cache_tracer_t(io::json _config, elfloader_t *_elf);
	~cache_tracer_t();
	bool interested(
		const working_set_t &ws, const insn_bits_t opc, const insn_t &insn) {
		return tracing;
	}
	void trace(
		const working_set_t &ws, const insn_bits_t opc, const insn_t &insn);
	void tabulate() {}
	io::json to_json() const;
protected:
	cache_t dcache;
	cache_t icache;
	bool got_host = false;
	bool tracing = true;
	addr_t tohost = 0;
	addr_t fromhost = 0;
protected:
	cycle_t last_access = 0;
	cycle_t last_write_back = 0;
	map_stat_t<uint32_t, counter_stat_t<uint32_t> *> write_back_histogram;
	map_stat_t<uint32_t, counter_stat_t<uint32_t> *> access_histogram;
	counter_stat_t<cycle_t> write_backs;
	counter_stat_t<cycle_t> cycles;
	counter_stat_t<cycle_t> accesses;
};

#endif