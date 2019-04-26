#ifndef PARSE_H
#define PARSE_H

#include <map>
#include <io/io.h>

class module_t;
std::map<std::string, std::function<module_t*()>> parse_modules(io::json config);

std::map<std::string, module_t *> parse_cxns(io::json config, 
	const std::map<std::string, std::function<module_t*()>>& module_types); 

#endif