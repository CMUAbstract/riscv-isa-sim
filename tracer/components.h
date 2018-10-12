#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "core.h"
#include "cores/simple_core.h"
#include "cores/si3stage_core.h"

#include "mem.h"
#include "cache/simple_mem.h"

const std::map<std::string, core_t*(*)(io::json, event_list_t *, mem_t *)> 
	core_type_map = {
	{"simple_core", &create_core<simple_core_t>},
	{"si3stage_core", &create_core<si3stage_core_t>}
};

const std::map<std::string, mem_t*(*)(io::json, event_list_t *)> mem_type_map = {
	{"simple_mem", &create_mem<simple_mem_t>}
};

#endif