#include "cache_tracer.h"

#include <algorithm>

#include <fesvr/elfloader.h>

#include "working_set.h"

#define SIGN_EXTEND(v) (0xFFFFFFFF00000000 | v)

uint32_t ilog2(uint32_t v) {
	uint32_t log2 = 0;
	while(v >>= 1) ++log2;
	return log2;
}

cache_tracer_t::cache_tracer_t(io::json _config, elfloader_t *_elf)
	: tracer_impl_t("cache_tracer", _config, _elf), 
	write_back_histogram("write_back"), access_histogram("access"), 
	write_backs("write_backs"), cycles("cycles"), accesses("accesses") {
	write_backs.reset();
	accesses.reset();
	cycles.reset();

	uint32_t block_size = 4;
	uint32_t ways = 2;
	uint32_t size = 256;

	JSON_CHECK(int, config["dcache"]["block_size"], block_size);
	JSON_CHECK(int, config["dcache"]["ways"], ways);
	JSON_CHECK(int, config["dcache"]["size"], size);

	dcache.init(block_size, ways, size);

	JSON_CHECK(int, config["icache"]["block_size"], block_size);
	JSON_CHECK(int, config["icache"]["ways"], ways);
	JSON_CHECK(int, config["icache"]["size"], size);

	icache.init(block_size, ways, size);
}

cache_tracer_t::~cache_tracer_t() {
	dump();
	for(auto it : write_back_histogram) delete it.second;
	for(auto it : access_histogram) delete it.second;
}

void cache_tracer_t::trace(
	const working_set_t &ws, const insn_bits_t opc, const insn_t &insn) {
	if(!got_host) {
		auto symbols = elf->get_symbols();
		auto it = symbols.find("tohost");
		if(it != symbols.end()) {
			tohost = SIGN_EXTEND(it->second.e32.st_value);
		}
		it = symbols.find("fromhost");
		if(it != symbols.end()) {
			fromhost = SIGN_EXTEND(it->second.e32.st_value);
		}
		got_host = true;
	}
	cycles.inc();
	addr_t insn_addr = SIGN_EXTEND(ws.pc);
	bool icache_miss, icache_evict, icache_write_back;
	std::tie(icache_miss, icache_evict, icache_write_back) = 
		icache.update(insn_addr, false);

	if(icache_miss) {
		accesses.inc();
		cycle_t diff = cycles.get() - last_access;
		auto hit = access_histogram.find(diff);
		if(hit == access_histogram.end()) {
			counter_stat_t<uint32_t> *counter_stat = 
				new counter_stat_t<uint32_t>();
			counter_stat->inc();
			access_histogram.insert(diff, counter_stat);
		} else {
			hit->second->inc();
		}
		last_access = cycles.get();
	}

	bool caused_access = false;
	bool caused_write_back = false;
	for(auto loc : ws.input.locs) {
		bool dcache_miss, dcache_evict, dcache_write_back;
		addr_t loc_addr = SIGN_EXTEND(loc);
		if(loc_addr == tohost || loc_addr == fromhost) tracing = false;
		std::tie(dcache_miss, dcache_evict, dcache_write_back) = 
			dcache.update(loc, false);
		if(dcache_write_back) {
			write_backs.inc();
			caused_write_back = true;
		}
		caused_access = !caused_access && dcache_miss;
	}

	for(auto loc : ws.output.locs) {
		bool dcache_miss, dcache_evict, dcache_write_back;
		addr_t loc_addr = SIGN_EXTEND(loc);
		std::tie(dcache_miss, dcache_evict, dcache_write_back) = 
			dcache.update(loc, true);
			if(loc_addr == tohost || loc_addr == fromhost) tracing = false;
		if(dcache_write_back) {
			write_backs.inc();
			caused_write_back = true;
		}
		caused_access = !caused_access && dcache_miss;
	}

	if(caused_write_back) {
		cycle_t diff = cycles.get() - last_write_back;
		auto hit = write_back_histogram.find(diff);
		if(hit == write_back_histogram.end()) {
			counter_stat_t<uint32_t> *counter_stat = 
				new counter_stat_t<uint32_t>();
			counter_stat->inc();
			write_back_histogram.insert(diff, counter_stat);
		} else {
			hit->second->inc();
		}
		last_write_back = cycles.get();
	}

	if(caused_access) {
		accesses.inc();
		cycle_t diff = cycles.get() - last_write_back;
		auto hit = access_histogram.find(diff);
		if(hit == access_histogram.end()) {
			counter_stat_t<uint32_t> *counter_stat = 
				new counter_stat_t<uint32_t>();
			counter_stat->inc();
			access_histogram.insert(diff, counter_stat);
		} else {
			hit->second->inc();
		}
		last_write_back = cycles.get();
	}
}

io::json cache_tracer_t::to_json() const {
	return io::json::merge_objects(access_histogram, write_back_histogram, 
		write_backs, accesses, cycles);
}

void cache_tracer_t::cache_t::init(
	uint32_t _block_size, uint32_t _ways, uint32_t _size) {
	block_size = _block_size;
	ways = _ways;
	size = _size;

	sets = (size / block_size) / ways;
	set_mask = (1 << ilog2(sets)) - 1;
	set_offset = 2 + ilog2(block_size); 

	tag_offset = set_offset + ilog2(sets);
	tag_mask = (1 << (32 - tag_offset)) - 1;
	// std::cout << "Set: " << set_mask << " " << set_offset << " ";
	// std::cout << sets << std::endl;
	// std::cout << "Tag: " << tag_mask << " " << tag_offset << std::endl;

	tag.resize(sets, std::vector<addr_t>(ways, 0));
	lru.resize(sets, std::vector<uint32_t>(ways, 0));
	valid.resize(sets, std::vector<bool>(ways, false));
	dirty.resize(sets, std::vector<bool>(ways, false));
}

std::tuple<bool, bool, bool> 
	cache_tracer_t::cache_t::update(addr_t addr, bool write) {
	uint32_t s = get_set(addr);
	// std::cout << "set: " << std::dec << s;
	// std::cout << " tag: " << std::hex << get_tag(addr) << std::endl;
	// for(auto it : valid) {
	// 	for(auto its : it) {
	// 		std::cout << its << " ";
	// 	}
	// 	std::cout << std::endl;
	// }
	// for(auto it : tag) {
	// 	for(auto its : it) {
	// 		std::cout << std::hex << its << " ";
	// 	}
	// 	std::cout << std::endl;
	// }

	size_t idx = 0;
	idx = std::distance(tag[s].begin(), 
		std::find(tag[s].begin(), tag[s].end(), get_tag(addr)));
	if(idx < ways) {
		for(auto & it : lru[s]) if(it > 0) it--;
		lru[s][idx] = ways;
		dirty[s][idx] = write;
		valid[s][idx] = true;
		// std::cout << "HIT " << idx << std::endl;
		// std::cout << "====================" << std::endl;
		return std::make_tuple(false, false, false);
	} 

	idx = std::distance(valid[s].begin(), 
		std::find(valid[s].begin(), valid[s].end(), false));
	if(idx < ways) {
		tag[s][idx] = get_tag(addr);
		lru[s][idx] = ways;
		dirty[s][idx] = write;
		valid[s][idx] = true;
		// std::cout << "HIT but on invalid " << idx << std::endl;
		// std::cout << "====================" << std::endl;
		return std::make_tuple(true, false, false);
	}
	
	idx = std::distance(lru[s].begin(), std::min_element(lru[s].begin(), lru[s].end()));
	// std::cout << "MISS " << idx << std::endl;
	// std::cout << "====================" << std::endl;
	bool d = dirty[s][idx];
	tag[s][idx] = get_tag(addr);
	lru[s][idx] = ways;
	dirty[s][idx] = write;
	valid[s][idx] = true;
	return std::make_tuple(true, true, d);
}