#include "vcu.h"

#include "mem_event.h"
#include "vector_event.h"

vcu_t::vcu_t(std::string _name, io::json _config, event_heap_t *_events)
	: component_t(_name, _config, _events) {
	pending_handler_t::set_ref(events, &clock);
	JSON_CHECK(int, config["lanes"], lanes, 1);
	JSON_CHECK(int, config["reg_count"], reg_count, 0x10);
	track_power("vrf");
	track_energy("alu");
	track_energy("reg_read");
	track_energy("reg_write");
}

void vcu_t::reset(reset_level_t level) {
	component_t::reset();
	outstanding = 0;
	empty = true;
}

io::json vcu_t::to_json() const {
	return component_t::to_json();
}

bool vcu_t::check_vec(insn_bits_t opc) {
	if((opc & 0x7F) == 0x57) return true;
	switch(opc) {
		case MATCH_VLH:
		case MATCH_VLXH:
		case MATCH_VLSH:
		case MATCH_VSH: 
		case MATCH_VSSH: 
		case MATCH_VSXH: return true;
	};
	return false;
}

bool vcu_t::check_split(insn_bits_t opc) {
	switch(opc) {
		case MATCH_VREDSUM_V:
		case MATCH_VPERMUTE_V:
		case MATCH_VSH: 
		case MATCH_VSSH: 
		case MATCH_VSXH: return true;
	}
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

void vcu_t::process(vec_start_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
}

void vcu_t::process(vec_reg_read_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
	count["reg_read"].running.inc();
}

void vcu_t::process(vec_reg_write_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
	count["reg_write"].running.inc();
}

void vcu_t::process(mem_ready_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
}

void vcu_t::process(mem_retire_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
}