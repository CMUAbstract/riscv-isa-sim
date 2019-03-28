#include "core.h"

#include "working_set.h"
#include "core_event.h"
#include "mem_event.h"

core_t::core_t(std::string _name, io::json _config, event_heap_t *_events)
	: component_t(_name, _config, _events), retired_insns("retired_insns", "") {
	JSON_CHECK(int, config["frequency"], frequency);
	
	// Statistics to track
	track_power("rf");
	track_power("exec");
	track_power("fetch_decode");
	track_energy("reg_read");
	track_energy("reg_write");
	track_energy("alu");
	track_energy("mul");
	track_energy("mem_req");
	track_energy("fetch");
	track_energy("decode");

	pending_handler_t::set_ref(events, &clock);
	squash_handler_t::set_ref(events);
	retired_insns.reset();
	stages["fetch"] = false;
	stages["decode"] = false;
	stages["exec"] = false;
}

void core_t::reset(reset_level_t level) {
	component_t::reset();
	clear_pending();
	clear_squash();
	retired_insns.reset();
	stages["fetch"] = false;
	stages["decode"] = false;
	stages["exec"] = false;
	insns.clear();
	insn_idx = 0;
	retired_idx = 0;
}

io::json core_t::to_json() const {
	return io::json::merge_objects(
		component_t::to_json(), retired_insns);
}

bool core_t::check_jump(insn_bits_t opc) {
	switch(opc) {
		case MATCH_JALR:
		case MATCH_C_J:
		case MATCH_C_JR:
		case MATCH_C_JAL:
		case MATCH_JAL:
		case MATCH_C_JALR: return true;
	};
	return false;
}

bool core_t::check_mul(insn_bits_t opc) {
	switch(opc) {
		case MATCH_MUL:
		case MATCH_MULH:
		case MATCH_MULHSU:
		case MATCH_MULHU:
		case MATCH_MULW:
		case MATCH_DIV: 
		case MATCH_DIVU: 
		case MATCH_DIVW: 
		case MATCH_DIVUW: return true;
	};
	return false;
}

void core_t::process(reg_read_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
	count["reg_read"].running.inc();
}

void core_t::process(reg_write_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
	count["reg_write"].running.inc();
}

void core_t::process(mem_ready_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
}

void core_t::process(mem_retire_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
}
