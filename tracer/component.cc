#include "component.h"

io::json component_t::to_json() const {
	return io::json(clock);
}