#include "curve_tracer.h"

#include <fesvr/elfloader.h>

#include "working_set.h"

#define SIGN_EXTEND(v) (0xFFFFFFFF00000000 | v)

void insn_curve_tracer_t::init(void) {
	bool elf32 = elf->check_elf32();
	auto text_section = elf->get_sections()[".text"];
	if(elf32) {
		text_base = SIGN_EXTEND(text_section.e32.sh_addr);
		text_size = text_section.e32.sh_size;
	} else {
		text_base = SIGN_EXTEND(text_section.e64.sh_addr);
		text_size = text_section.e64.sh_size;
	}
}

void insn_curve_tracer_t::trace(
	const working_set_t &ws, const insn_bits_t opc, const insn_t &insn) {
	auto insn_count = minstret++;
	for(auto loc : ws.output.locs) {
		if(loc >= text_base && loc <= text_base + text_size) continue;
		auto it = tracked_locations.find(loc);
		if(it == tracked_locations.end()) {
			tracked_locations.insert(std::make_pair(loc, insn_count));
			continue;
		}
		size_t diff = insn_count - it->second;
		tracked_locations[loc] = insn_count;
		auto hit = histogram.find(diff);
		if(hit == histogram.end()) {
			counter_stat_t<size_t> *counter_stat = new counter_stat_t<size_t>();
			counter_stat->inc();
			histogram.insert(diff, counter_stat);
			continue;
		}
		hit->second->inc();
	}
	for(auto loc : ws.input.locs) {
		if(loc >= text_base && loc <= text_base+ text_size) continue;
		auto it = tracked_locations.find(loc);
		if(it == tracked_locations.end()) {
			tracked_locations.insert(std::make_pair(loc, insn_count));
			continue;
		}
		size_t diff = insn_count - it->second;
		tracked_locations[loc] = insn_count;
		auto hit = histogram.find(diff);
		if(hit == histogram.end()) {
			counter_stat_t<size_t> *counter_stat = new counter_stat_t<size_t>();
			counter_stat->inc();
			histogram.insert(diff, counter_stat);
			continue;
		}
		hit->second->inc();
	}
}

insn_curve_tracer_t::~insn_curve_tracer_t() {
	dump();
	for(auto it : histogram) delete it.second;
}

void miss_curve_tracer_t::trace(
	const working_set_t &ws, const insn_bits_t opc, const insn_t &insn) {
	auto access_count = maccess;
	maccess += ws.output.locs.size() + ws.input.locs.size();
	for(auto loc : ws.output.locs) {
		if(loc >= text_base && loc <= text_base + text_size) continue;
		auto it = tracked_locations.find(loc);
		if(it == tracked_locations.end()) {
			tracked_locations.insert(std::make_pair(loc, access_count));
			continue;
		}
		size_t diff = access_count - it->second;
		tracked_locations[loc] = access_count;
		auto hit = histogram.find(diff);
		if(hit == histogram.end()) {
			counter_stat_t<size_t> *counter_stat = new counter_stat_t<size_t>();
			counter_stat->inc();
			histogram.insert(diff, counter_stat);
			continue;
		}
		hit->second->inc();
	}
	for(auto loc : ws.input.locs) {
		if(loc >= text_base && loc <= text_base+ text_size) continue;
		auto it = tracked_locations.find(loc);
		if(it == tracked_locations.end()) {
			tracked_locations.insert(std::make_pair(loc, access_count));
			continue;
		}
		size_t diff = access_count - it->second;
		tracked_locations[loc] = access_count;
		auto hit = histogram.find(diff);
		if(hit == histogram.end()) {
			counter_stat_t<size_t> *counter_stat = new counter_stat_t<size_t>();
			counter_stat->inc();
			histogram.insert(diff, counter_stat);
			continue;
		}
		hit->second->inc();
	}
}