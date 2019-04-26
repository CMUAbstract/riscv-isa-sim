#include "parse.h"

#include <string>

#include "modules.h"
#include "log.h"

std::map<std::string, std::function<module_t*()>> parse_modules(io::json config) {
	std::map<std::string, std::function<module_t*()>> module_types;
	for(auto it : config.object_items()) {
		assert_msg(it.second.is_object(), "%s has no config", it.first.c_str());
		assert_msg(it.second["type"].is_string(), "%s has no type", it.first.c_str());
		std::string name = it.first;
		std::string type = it.second["type"].string_value();
		if(type.compare("composite")) {

		} else {
			
		}
		// module_types.insert({name, module_type_map[type](name, it.second, scheduler)});
	}
	return module_types;
}

std::tuple<std::string, std::string> parse_cxn(std::string cxn) {
	auto it = cxn.find("::");
	assert_msg(it != cxn.size(), "Invalid connection (%s)", cxn.c_str());
	return std::make_tuple(cxn.substr(0, it), cxn.substr(it, cxn.size()));
}

std::map<std::string, module_t *> 
parse_cxns(io::json config, 
	const std::map<std::string, std::function<module_t*()>>& module_types) {
	std::map<std::string, module_t *> modules;
	for(auto it : config.object_items()) {
		auto from = parse_cxn(it.first);	

		// std::tuple to = parse_cxn(it.second.string_value());
	}
	return modules;
}