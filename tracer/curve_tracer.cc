#include "curve_tracer.h"

#include <iterator>

#include <fesvr/elfloader.h>

#include "log.h"
#include "working_set.h"

#define SIGN_EXTEND(v) (0xFFFFFFFF00000000 | v)

insn_curve_tracer_t::insn_curve_tracer_t(io::json _config, elfloader_t *_elf)
	: tracer_impl_t("insn_curve_tracer", _config, _elf), histogram("histogram"), 
	minstret("minstret") {
	minstret.reset();
}

void insn_curve_tracer_t::trace(
	const working_set_t &ws, const insn_bits_t opc, const insn_t &insn) {
	if(!text_calc) {
		bool elf32 = elf->check_elf32();
		auto text_section = elf->get_sections()[".text"];
		if(elf32) {
			text_base = SIGN_EXTEND(text_section.e32.sh_addr);
			text_size = text_section.e32.sh_size;
		} else {
			text_base = SIGN_EXTEND(text_section.e64.sh_addr);
			text_size = text_section.e64.sh_size;
		}
		text_calc = true;
	}
	
	auto insn_count = minstret.get();
	minstret.inc();

	for(auto loc : ws.output.locs) {
		if(loc >= text_base && loc <= text_base + text_size) continue;
		auto it = tracked_locations.find(loc);
		if(it == tracked_locations.end()) {
			tracked_locations.insert(std::make_pair(loc, insn_count));
			continue;
		}
		uint32_t diff = insn_count - it->second;
		tracked_locations[loc] = insn_count;
		auto hit = histogram.find(diff);
		if(hit == histogram.end()) {
			counter_stat_t<uint32_t> *counter_stat = new counter_stat_t<uint32_t>();
			counter_stat->inc();
			histogram.insert(diff, counter_stat);
			continue;
		}
		hit->second->inc();
	}

	for(auto loc : ws.input.locs) {
		if(loc >= text_base && loc <= text_base + text_size) continue;
		auto it = tracked_locations.find(loc);
		if(it == tracked_locations.end()) {
			tracked_locations.insert(std::make_pair(loc, insn_count));
			continue;
		}
		uint32_t diff = insn_count - it->second;
		tracked_locations[loc] = insn_count;
		auto hit = histogram.find(diff);
		if(hit == histogram.end()) {
			counter_stat_t<uint32_t> *counter_stat = new counter_stat_t<uint32_t>();
			counter_stat->inc();
			histogram.insert(diff, counter_stat);
			continue;
		}
		hit->second->inc();
	}
}

insn_curve_tracer_t::~insn_curve_tracer_t() {
	if(!serialized) dump();
	for(auto it : histogram) delete it.second;
}

io::json insn_curve_tracer_t::to_json() const {
	return io::json::merge_objects(histogram, minstret);
}

miss_curve_tracer_t::miss_curve_tracer_t(io::json _config, elfloader_t *_elf) 
	: insn_curve_tracer_t(_config, _elf), 
	maccess("maccess"), cold_misses("cold_misses") { 
	name = "miss_curve_tracer";
	maccess.reset();
	cold_misses.reset();
	JSON_CHECK(bool, config["lru"], lru);
	JSON_CHECK(bool, config["exclude_text"], exclude_text);
	JSON_CHECK(int, config["line_size"], line_size);
	line_mask = ~(line_size - 1);	
}

miss_curve_tracer_t::~miss_curve_tracer_t() {
	counter_stat_t<uint32_t> *counter_stat = new counter_stat_t<uint32_t>();
	counter_stat->set(cold_misses.get());
	histogram.insert(max_dist + 1, counter_stat);
	dump();
	serialized = true;
}

void miss_curve_tracer_t::trace(
	const working_set_t &ws, const insn_bits_t opc, const insn_t &insn) {
	if(!text_calc) {
		bool elf32 = elf->check_elf32();
		auto text_section = elf->get_sections()[".text"];
		if(elf32) {
			text_base = SIGN_EXTEND(text_section.e32.sh_addr);
			text_size = text_section.e32.sh_size;
		} else {
			text_base = SIGN_EXTEND(text_section.e64.sh_addr);
			text_size = text_section.e64.sh_size;
		}
		text_calc = true;
	}

	minstret.inc();
	if(lru) {
		for(auto loc : ws.output.locs) {
			if(exclude_text && SIGN_EXTEND(loc) >= text_base && 
				SIGN_EXTEND(loc) <= text_base + text_size) {
				continue;
			} else if(exclude_text && SIGN_EXTEND(loc) < text_base) continue;
			loc = loc & line_mask;
			auto it = tracked_locations.find(loc);
			if(it != tracked_locations.end()) {
				assert_msg(*indices[loc] == loc, "Iterator incorrect");
				uint32_t dist = std::distance(dq.begin(), indices[loc]);
				if(dist > max_dist) max_dist = dist;
				auto hit = histogram.find(dist);
				if(hit == histogram.end()) {
					counter_stat_t<uint32_t> *counter_stat = 
						new counter_stat_t<uint32_t>();
					counter_stat->reset();
					histogram.insert(dist, counter_stat);
				}
				histogram[dist]->inc();
			} else {
				tracked_locations[loc] = 0;
				cold_misses.inc();
			}
			if(indices.find(loc) != indices.end()) dq.erase(indices[loc]); 
			dq.push_front(loc);
			indices[loc] = dq.begin();
			maccess.inc();
		}

		for(auto loc : ws.input.locs) {
			if(exclude_text && SIGN_EXTEND(loc) >= text_base && 
				SIGN_EXTEND(loc) <= text_base + text_size) {
				continue;
			} else if(exclude_text && SIGN_EXTEND(loc) < text_base) continue;
			loc = loc & line_mask;
			auto it = tracked_locations.find(loc);
			if(it != tracked_locations.end()) {
				assert_msg(*indices[loc] == loc, "Iterator incorrect");
				uint32_t dist = std::distance(dq.begin(), indices[loc]);
				if(dist > max_dist) max_dist = dist;
				auto hit = histogram.find(dist);
				if(hit == histogram.end()) {
					counter_stat_t<uint32_t> *counter_stat = 
						new counter_stat_t<uint32_t>();
					counter_stat->reset();
					histogram.insert(dist, counter_stat);
				}
				histogram[dist]->inc();
			} else {
				tracked_locations[loc] = 0;
				cold_misses.inc();
			}
			if(indices.find(loc) != indices.end()) dq.erase(indices[loc]); 
			dq.push_front(loc);
			indices[loc] = dq.begin();
			maccess.inc();
		}
		return;
	}

	maccess.inc(ws.output.locs.size() + ws.input.locs.size());
	auto access_count = maccess.get();
	for(auto loc : ws.output.locs) {
		if(exclude_text && SIGN_EXTEND(loc) >= text_base && 
			SIGN_EXTEND(loc) <= text_base + text_size) continue;
		auto it = tracked_locations.find(loc);
		if(it == tracked_locations.end()) {
			tracked_locations.insert(std::make_pair(loc, access_count));
			continue;
		}
		uint32_t diff = access_count - it->second;
		tracked_locations[loc] = access_count;
		auto hit = histogram.find(diff);
		if(hit == histogram.end()) {
			counter_stat_t<uint32_t> *counter_stat = 
				new counter_stat_t<uint32_t>();
			counter_stat->inc();
			histogram.insert(diff, counter_stat);
			continue;
		}
		hit->second->inc();
	}

	for(auto loc : ws.input.locs) {
		if(exclude_text && SIGN_EXTEND(loc) >= text_base && 
			SIGN_EXTEND(loc) <= text_base + text_size) continue;
		auto it = tracked_locations.find(loc);
		if(it == tracked_locations.end()) {
			tracked_locations.insert(std::make_pair(loc, access_count));
			continue;
		}
		uint32_t diff = access_count - it->second;
		tracked_locations[loc] = access_count;
		auto hit = histogram.find(diff);
		if(hit == histogram.end()) {
			counter_stat_t<uint32_t> *counter_stat = 
				new counter_stat_t<uint32_t>();
			counter_stat->inc();
			histogram.insert(diff, counter_stat);
			continue;
		}
		hit->second->inc();
	}
}

io::json miss_curve_tracer_t::to_json() const {
	auto merged = io::json::merge_objects(insn_curve_tracer_t::to_json(), 
		maccess, cold_misses);
	return merged;
}
