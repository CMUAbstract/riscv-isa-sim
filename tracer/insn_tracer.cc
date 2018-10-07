#include "insn_tracer.h"

#include "working_set.h"
#include "insn_detail.h"

std::map<insn_bits_t, uint64_t> insn_tracer_t::m = std::map<insn_bits_t, uint64_t>();
bool insn_tracer_t::insn_registered = false;

void insn_tracer_t::register_insn_types(void) {
	#define DEFINE_INSN_STAT(opc, type) m.insert(std::make_pair(opc, type)); 
	#include "insn_stat.h"	
	#undef DEFINE_INSN_STAT
}

std::string perf_tracer_t::dump() {
	return io::json(mcycles).dump();
}

void perf_tracer_t::trace(working_set_t *ws, insn_bits_t opc, insn_t insn) {
	/*reg_t vl = p->get_state()->vl;
	switch(m[opc]) {
		case LOAD: return mcycles.inc(load_cycles); 
		case STORE: return mcycles.inc(store_cycles);
		case CONTROL: return mcycles.inc(control_cycles);
		case ARITH0: return mcycles.inc(arith0_cycles);
		case ARITH1: return mcycles.inc(arith1_cycles);
		case ARITH2: return mcycles.inc(arith2_cycles);
		case ARITH3: return mcycles.inc(arith3_cycles);
		case VLOAD: 
			return mcycles.inc(control_cycles + (vl / vamortization) * vload_cycles);
		case VSTORE: 
			return mcycles.inc(control_cycles + (vl / vamortization) * vstore_cycles);
		case VARITH0: 
			return mcycles.inc(control_cycles + (vl / vamortization) * varith0_cycles);
		case VARITH1: 
			return mcycles.inc(control_cycles + (vl / vamortization) * varith1_cycles);
		default: break;
	}*/
}

void energy_tracer_t::trace(working_set_t *ws, insn_bits_t opc, insn_t insn) {
	/*reg_t vl = p->get_state()->vl;
	switch(insn_tracer_t::m[opc]) {
		case LOAD: return menergy.inc(load_energy); 
		case STORE: return menergy.inc(store_energy);
		case CONTROL: return menergy.inc(control_energy);
		case ARITH0: return menergy.inc(arith0_energy);
		case ARITH1: return menergy.inc(arith1_energy);
		case ARITH2: return menergy.inc(arith2_energy);
		case ARITH3: return menergy.inc(arith3_energy);
		case VLOAD: return menergy.inc(control_energy + vl * vload_energy);
		case VSTORE: return menergy.inc(control_energy + vl * vstore_energy);
		case VARITH0: return menergy.inc(control_energy + vl * varith0_energy);
		case VARITH1: return menergy.inc(control_energy + vl * varith1_energy);
		default: break;
	}*/
}

std::string energy_tracer_t::dump() {
	return io::json(menergy).dump();
}
