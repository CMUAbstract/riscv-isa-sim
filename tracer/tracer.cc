#include "tracer.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cerrno>

#include "log.h"

#include "basic_tracer.h"
#include "curve_tracer.h"
#include "insn_tracer.h"
#include "time_tracer.h"

template<typename T> tracer_t* create_tracer(io::json config, elfloader_t *elf) { 
	return new T(config, elf);
}

std::map<std::string, tracer_t*(*)(io::json, elfloader_t *)> tracer_type_map = {
	{"basic_ram_tracer", &create_tracer<basic_ram_tracer_t>},
	{"insn_curve_tracer", &create_tracer<insn_curve_tracer_t>},
	{"miss_curve_tracer", &create_tracer<miss_curve_tracer_t>},
	{"perf_tracer", &create_tracer<perf_tracer_t>},
	{"energy_tracer", &create_tracer<energy_tracer_t>},
	{"time_tracer", &create_tracer<time_tracer_t>},
};

core_tracer_t::core_tracer_t(std::string _config, elfloader_t *_elf)
	: tracer_list_t(nullptr, _elf) {
	std::ifstream in(_config, std::ios::in | std::ios::binary);
	if(in) {
    	std::ostringstream contents;
    	contents << in.rdbuf();
    	in.close();
    	std::string err;
		config = io::json::parse(contents.str(), err);
		assert_msg(config != nullptr, "Config is null (%s)", err.c_str());
		init();
		return;
	}
}

void core_tracer_t::init() {
	for(auto it : config.object_items()) {
		assert_msg(tracer_type_map.find(it.first) != tracer_type_map.end(), 
			"%s not found", it.first.c_str());
		list.push_back(tracer_type_map[it.first](it.second, elf));
	}
}