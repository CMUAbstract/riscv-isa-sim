#include "modules.h"

std::function<module_t *()> 
create_module_type(std::string type, 
	std::string name, io::json config, scheduler_t *scheduler) {
	return [type, name, config, scheduler](){ 
		return module_map.at(type)(name, config, scheduler); 
	};
}