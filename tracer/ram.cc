#include "ram.h"

ram_t::ram_t(std::string _name, io::json _config, event_list_t *_events) 
	: component_t(_name, _config, _events), reads("reads"), writes("writes") {
	reads.reset();
	writes.reset();
}

io::json ram_t::to_json() const {
	return component_t::to_json();
}