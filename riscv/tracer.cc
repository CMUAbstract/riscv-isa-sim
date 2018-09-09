#include "tracer.h"

#include <iostream>

basic_mem_tracer_t::~basic_mem_tracer_t() {
	std::cerr << tracked_locations.dump();
	for(auto it : tracked_locations) delete it.second;
}

void basic_mem_tracer_t::trace(
	processor_t *p, insn_bits_t opc, insn_t insn, working_set_t ws) {
	for(auto loc : ws.input.locs) {
		auto it = tracked_locations.find(loc);
		if(it == tracked_locations.end()) {
			mem_loc_stat_t *mem_loc_stat = new mem_loc_stat_t();
			mem_loc_stat->reads.inc();
			tracked_locations.insert(loc, mem_loc_stat);
			continue;
		}
		it->second->reads.inc();
	}
	for(auto loc : ws.output.locs) {
		auto it = tracked_locations.find(loc);
		if(it == tracked_locations.end()) {
			mem_loc_stat_t *mem_loc_stat = new mem_loc_stat_t();
			mem_loc_stat->writes.inc();
			tracked_locations.insert(loc, mem_loc_stat);
			continue;
		}
		it->second->writes.inc();
	}
}

miss_curve_tracer_t::~miss_curve_tracer_t() {
	std::cerr << histogram.dump();
	for(auto it : histogram) delete it.second;
}

void miss_curve_tracer_t::trace(
	processor_t *p, insn_bits_t opc, insn_t insn, working_set_t ws) {
	auto insn_count = minstret++;
	for(auto loc : ws.output.locs) {
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
