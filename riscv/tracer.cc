#include "tracer.h"
#include "sim.h"
#include <fesvr/elfloader.h>

#include <iostream>

#define SIGN_EXTEND(v) (0xFFFFFFFF00000000 | v)

basic_mem_tracer_t::~basic_mem_tracer_t() {
	std::map<std::string, region_t> symbols;
	std::map<std::string, region_t> sections;
	for(auto sym : elf->get_symbols()) {
		region_t tmp(0, 0);
		if(elf->check_elf32()) {
			tmp.base = SIGN_EXTEND(sym.second.e32.st_value);
			tmp.size = sym.second.e32.st_size;
		} else {
			tmp.base = SIGN_EXTEND(sym.second.e64.st_value);
			tmp.size = sym.second.e64.st_size;
		}
		symbols[sym.first] = tmp;
	}
	for(auto sec : elf->get_sections()) {
		region_t tmp(0, 0);
		if(elf->check_elf32()) {
			tmp.base = SIGN_EXTEND(sec.second.e32.sh_addr);
			tmp.size = sec.second.e32.sh_size;
		} else {
			tmp.base = SIGN_EXTEND(sec.second.e64.sh_addr);
			tmp.size = sec.second.e64.sh_size;
		}
		sections[sec.first] = tmp;
	}
	for(auto &it : tracked_locations) {
		for(auto s : symbols) {
			if(s.second.base <= it.first && 
				s.second.base + s.second.size >= it.first + 1){
				it.second->symbol = s.first;
			}
		}
		for(auto s : sections) {
			if(s.second.base <= it.first && 
				s.second.base + s.second.size >= it.first + 1){
				it.second->section = s.first;
			}
		}
	}
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

std::string basic_mem_tracer_t::mem_loc_stat_t::dump(void) const {
	std::ostringstream os;
	os << "{\"symbol\": \"" << symbol << "\",";
	os << "\"section\": \"" << section << "\",";
	os << reads.dump() << ",";
	os << writes.dump() << "}";
	return os.str();
}

miss_curve_tracer_t::miss_curve_tracer_t(elfloader_t *_elf) : tracer_t(), elf(_elf) {
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

miss_curve_tracer_t::~miss_curve_tracer_t() {
	std::cerr << histogram.dump();
	for(auto it : histogram) delete it.second;
}

void miss_curve_tracer_t::trace(
	processor_t *p, insn_bits_t opc, insn_t insn, working_set_t ws) {
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
