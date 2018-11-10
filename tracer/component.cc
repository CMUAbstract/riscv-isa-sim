#include "component.h"

io::json component_t::to_json() const {
	std::cout << "COMP" << std::endl;
	return io::json::object{{name, clock}};
}