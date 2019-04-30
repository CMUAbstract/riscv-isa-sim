#ifndef PARSE_H
#define PARSE_H

#include <map>
#include <io/io.h>

#include "modules.h"

class module_t;
class scheduler_t;
std::map<std::string, module_creator_t> parse_types(io::json config);

std::map<std::string, module_t *> parse_cxns(module_t *module, io::json config, 
	const std::map<std::string, module_creator_t>& types, scheduler_t *scheduler); 

#endif