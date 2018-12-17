#include "tracer.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cerrno>
#include <fstream>
#include <experimental/filesystem>

#include "log.h"

#include "basic_tracer.h"
#include "curve_tracer.h"
#include "insn_tracer.h"
#include "time_tracer.h"

tracer_impl_t::tracer_impl_t(std::string _name, io::json _config, elfloader_t *_elf) 
	: tracer_t(), name(_name), config(_config), elf(_elf) {
	JSON_CHECK(string, config["outdir"], outdir);	
}

io::json tracer_impl_t::to_json() const {
	return io::json::object{{name, nullptr}};
}

void tracer_impl_t::dump() {
	if(outdir.size()) {
		std::experimental::filesystem::path dir(outdir);
		std::string file_ext = ".json";
		std::experimental::filesystem::path file(name + file_ext);
		std::experimental::filesystem::path p = dir / file;
		std::ofstream o;
		o.open(p);
		o << to_json().dump();
		o.close();
	}
}

#define SIGN_EXTEND(v) (0xFFFFFFFF00000000 | v)
bool tracer_list_t::interested(
	const working_set_t &ws, const insn_bits_t opc, const insn_t &insn) {
	if(ws.pc == SIGN_EXTEND(roi_start)) set_hyperdrive(false);
	else if(ws.pc == SIGN_EXTEND(roi_end)) set_hyperdrive(true);
	for(auto it : list)
		if (it->interested(ws, opc, insn))
			return !hyperdrive && true;
	return false;
}

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
	name = "core_tracer";
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