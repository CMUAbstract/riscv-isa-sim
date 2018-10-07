#include "tracer.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cerrno>

#include "basic_tracer.h"
#include "curve_tracer.h"
#include "insn_tracer.h"
// #include "time_tracer.h"

template<typename T> tracer_t* create_tracer(io::json config, elfloader_t *elf) { 
	return new T(config, elf);
}

std::map<std::string, tracer_t*(*)(io::json, elfloader_t *)> tracer_type_map = {
	{"basic_mem_tracer", &create_tracer<basic_mem_tracer_t>},
	{"insn_curve_tracer_t", &create_tracer<insn_curve_tracer_t>},
	{"miss_curve_tracer_t", &create_tracer<miss_curve_tracer_t>},
	{"perf_tracer_t", &create_tracer<perf_tracer_t>},
	{"energy_tracer_t", &create_tracer<energy_tracer_t>},
};

core_tracer_t::core_tracer_t(std::string _config, elfloader_t *_elf)
	: tracer_impl_t(nullptr, _elf) {
	std::ifstream in(_config, std::ios::in | std::ios::binary);
	if(in) {
    	std::ostringstream contents;
    	contents << in.rdbuf();
    	in.close();
    	std::string err;
		config = io::json::parse(contents.str(), err);
		if(config == nullptr) throw err;
		init();
		return;
	}
	throw(errno);
}

void core_tracer_t::init() {
	for(auto it : config.object_items()) {
		tracers.push_back(tracer_type_map[it.first](it.second, elf));
	}
}