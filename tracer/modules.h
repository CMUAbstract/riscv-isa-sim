#ifndef MODULES_H
#define MODULES_H

#include "module.h"

#include "core/fetch.h"
#include "core/decode.h"
#include "core/exec.h"
#include "core/retire.h"

// #include "ram/cache.h"
// #include "ram/main.h"

// #include "vector/vec1d.h"
// #include "vector/vec1dflow.h"

typedef module_t*(*create_module_t)(std::string, io::json, scheduler_t *);

template<typename T> 
module_t* create_module(
	std::string name, io::json config, scheduler_t *scheduler) {
	return new T(name, config, scheduler);
};

std::function<module_t *()> 
create_module_type(std::string type, 
	std::string name, io::json config, scheduler_t *scheduler);

const std::map<std::string, create_module_t> 
module_map = {
	{"composite", &create_module<composite_t>},
	// Core stages
	{"fetch", &create_module<fetch_t>},
	{"decode", &create_module<decode_t>},
	{"exec", &create_module<exec_t>},
	{"retire", &create_module<retire_t>}
	// Memory elements
	// {"cache", &create_module<cache_t>},
	// {"main", &create_module<main_t>},
	// Branch predictors
	// {"local", &create_module<local_branch_predictor_t>},
	// {"global", &create_module<global_branch_predictor_t>},
	// {"tournament", &create_module<tournament_branch_predictor_t>},
	// Replacement policies
	// {"local", &create_module<lru_repl_policy_t}//,
	// VCUs
	// {"vec1d", &create_module<vec1d_t>},
	// {"vec1dflow", &create_module<vec1dflow_t>},
};

#endif