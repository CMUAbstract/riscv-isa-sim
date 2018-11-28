#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "core.h"
// #include "cores/simple_core.h"
#include "cores/si3stage_core.h"

#include "ram.h"
// #include "cache/simple_ram.h"
#include "cache/cache.h"
#include "cache/main_mem.h"

template<typename T, typename K> 
T* create_component(std::string name, io::json config, event_hmap_t *events) {
	return new K(name, config, events);
}

const std::map<std::string, core_t*(*)(std::string name, io::json, event_hmap_t *)> 
core_type_map = {
	// {"simple_core", &create_core<simple_core_t>},
	{"si3stage_core", &create_component<core_t, si3stage_core_t>}
};

const std::map<std::string, ram_t*(*)(std::string name, io::json, event_hmap_t *)> 
ram_type_map = {
	// {"simple_mem", &create_mem<simple_ram_t>},
	{"cache", &create_component<ram_t, cache_t>},
	{"main", &create_component<ram_t, main_mem_t>}
};

#endif