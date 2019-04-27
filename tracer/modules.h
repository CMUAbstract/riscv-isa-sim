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

typedef std::function<module_t*(std::string name, scheduler_t *scheduler)> module_creator_t;

template<typename T>
module_creator_t create_module_type(io::json config) {
	return [config](std::string name, scheduler_t *scheduler) {
		return new T(name, config, scheduler);
	};
}

const std::map<std::string, module_creator_t(*)(io::json)> 
module_type_map = {
	// Core stages
	{"composite", &create_module_type<composite_t>},
	{"fetch", &create_module_type<fetch_t>},
	{"decode", &create_module_type<decode_t>},
	{"exec", &create_module_type<exec_t>},
	{"retire", &create_module_type<retire_t>}
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