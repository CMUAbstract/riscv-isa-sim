#include "vcu.h"

vcu_t::vcu_t(std::string _name, io::json _config, event_heap_t *_events)
	: component_t(_name, _config, _events) {
	pending_handler_t::set_ref(events);
	JSON_CHECK(int, config["line_size"], line_size);
	JSON_CHECK(int, config["lines"], lines);
	JSON_CHECK(int, config["reg_count"], reg_count);
}

io::json vcu_t::to_json() const {
	return component_t::to_json();
}

bool vcu_t::check_vec(insn_bits_t opc) {
	switch(opc) {
		case MATCH_VADD:
		case MATCH_VMUL:
		case MATCH_VREDSUM:
		case MATCH_VLH:
		case MATCH_VLXH:
		case MATCH_VSH: return true;
	};
	return false;
}

void vcu_t::check_and_set_vl(hstd::shared_ptr<timed_insn_t> insn) {
	if(insn->opc != MATCH_VSETVL) return;
	for(auto csr : insn->ws.output.csrs) {
		if(std::get<0>(csr) == CSR_VL) {
			vl = std::get<1>(csr);
			break;
		}
	}
}