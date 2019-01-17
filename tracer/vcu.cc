#include "vcu.h"

#include "mem_event.h"
#include "vector_event.h"

vcu_t::vcu_t(std::string _name, io::json _config, event_heap_t *_events)
	: component_t(_name, _config, _events) {
	pending_handler_t::set_ref(events);
	JSON_CHECK(int, config["lanes"], lanes, 1);
}

io::json vcu_t::to_json() const {
	return component_t::to_json();
}

bool vcu_t::check_vec(insn_bits_t opc) {
	switch(opc) {
		case MATCH_VADD:
		case MATCH_VSUB:
		case MATCH_AND:
		case MATCH_VMUL:
		case MATCH_VCLIPH:
		case MATCH_VREDSUM:
		case MATCH_VPERMUTE:
		case MATCH_VLH:
		case MATCH_VLXH:
		case MATCH_VLSH:
		case MATCH_VSH: 
		case MATCH_VSSH: return true;
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

void vcu_t::process(vector_reg_read_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
}

void vcu_t::process(vector_reg_write_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
}

void vcu_t::process(mem_ready_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
}

void vcu_t::process(mem_retire_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
}

void vcu_t::process(mem_match_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
}