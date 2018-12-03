#include "vcu.h"

vcu_t::vcu_t(std::string _name, io::json _config, event_heap_t *_events)
	: component_t(_name, _config, _events) {
	pending_handler_t::set_ref(events);
}

io::json vcu_t::to_json() const {
	return component_t::to_json();
}

bool vcu_t::check_vec(insn_bits_t opc) {
	return true;
}