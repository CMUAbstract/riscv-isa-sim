#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "core.h"
#include "cores/si3stage_core.h"

#include "ram.h"
#include "cache/cache.h"
#include "cache/main_mem.h"

#include "vcu.h"
#include "vector/vec1d.h"
#include "vector/vecflow.h"

template<typename T, typename K> 
T* create_component(std::string name, io::json config, event_heap_t *events) {
	return new K(name, config, events);
}

const std::map<std::string, core_t*(*)(std::string name, io::json, event_heap_t *)> 
core_type_map = {
	{"si3stage_core", &create_component<core_t, si3stage_core_t>}
};

const std::map<std::string, vcu_t*(*)(std::string name, io::json, event_heap_t *)> 
vcu_type_map = {
	{"vec1d", &create_component<vcu_t, vec1d_t>},
	{"vecflow", &create_component<vcu_t, vecflow_t>}
};

const std::map<std::string, ram_t*(*)(std::string name, io::json, event_heap_t *)> 
ram_type_map = {
	{"cache", &create_component<ram_t, cache_t>},
	{"main", &create_component<ram_t, main_mem_t>}
};

#endif