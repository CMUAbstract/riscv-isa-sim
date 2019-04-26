#include "vector_tracer.h"

mask_tracer_t::mask_tracer_t(io::json _config, elfloader_t *_elf)
	: tracer_impl_t("mask_tracer", _config, _elf), insn_count("insn_count"), 
	mask_count("mask_count"), total_lanes("total_lanes"), masked_lanes("masked_lanes") {
	insn_count.reset();
	mask_count.reset();
	total_lanes.reset();
	masked_lanes.reset();
	for(auto &it : vregs) it.fill(0);
}

bool mask_tracer_t::interested(
		const working_set_t &ws, const insn_bits_t opc, const insn_t &insn) {
	if(opc == MATCH_VSETVL) {
		for(auto csr : ws.output.csrs) {
			if(std::get<0>(csr) == CSR_VL) {
				vl = std::get<1>(csr);
				break;
			}
		}
		return false;
	}
	bool is_vec = false;
	if((opc & 0x7F) == 0x57) is_vec = true;
	switch(opc) {
		case MATCH_VLH:
		case MATCH_VLXH:
		case MATCH_VLSH:
		case MATCH_VSH: 
		case MATCH_VSSH: 
		case MATCH_VSXH: is_vec = true;
	}
	if(is_vec) {
		for(auto it : ws.diff.vregs) {
			uint8_t reg = std::get<0>(it);
			uint8_t pos = std::get<1>(it);
			uint16_t val = std::get<2>(it);
			vregs[reg][pos] = val;	
		}
	}
	return is_vec;
}

void mask_tracer_t::trace(
	const working_set_t &ws, const insn_bits_t opc, const insn_t &insn) {
	insn_count.inc();
	total_lanes.inc(vl);
	if(insn.vm()) {
		mask_count.inc();
		for(uint16_t i = 0; i < vl; i++)
			if((vregs[0][i] & 0x1) == 0) masked_lanes.inc();
	} else if(opc == MATCH_VMV_V) {
		mask_count.inc();
		uint16_t mvreg = insn.rs2() & 0xF;
		for(uint16_t i = 0; i < vl; i++)
			if((vregs[mvreg][i] & 0x1) == 0) masked_lanes.inc();	
	}
}

io::json mask_tracer_t::to_json() const {
	return io::json::merge_objects(insn_count, mask_count, total_lanes, masked_lanes);
}