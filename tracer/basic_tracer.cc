#include "basic_tracer.h"

#include <fesvr/elfloader.h>

#include "working_set.h"

#define SIGN_EXTEND(v) (0xFFFFFFFF00000000 | v)

void basic_ram_tracer_t::trace(working_set_t *ws, insn_bits_t opc, insn_t insn) {
	for(auto loc : ws->input.locs) {
		auto it = tracked_locations.find(loc);
		if(it == tracked_locations.end()) {
			mem_loc_stat_t *mem_loc_stat = new mem_loc_stat_t();
			mem_loc_stat->reads.inc();
			tracked_locations.insert(loc, mem_loc_stat);
			continue;
		}
		it->second->reads.inc();
	}
	for(auto loc : ws->output.locs) {
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

void basic_ram_tracer_t::tabulate() {
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
}

basic_ram_tracer_t::~basic_ram_tracer_t() {
	dump();
	for(auto it : tracked_locations) delete it.second;
}