#include "vector_tracer.h"

vector_tracer_t::vector_tracer_t(io::json _config, elfloader_t *_elf)
	: tracer_impl_t("vector_tracer", _config, _elf), repeats("repeats"),
	thru_mem("thru_mem"), vcount("vcount") {
	repeats.reset();
	thru_mem.reset();
	vcount.reset();
}

vector_tracer_t::~vector_tracer_t() {
	dump();
	for(auto it : vinsns) delete it;
}

bool vector_tracer_t::interested(
		const working_set_t &ws, const insn_bits_t opc, const insn_t &insn) {
	switch(opc) {
		case MATCH_VSETVLB:
		case MATCH_VSETVLH:
		case MATCH_VSETVLW: {
			for(auto csr : ws.output.csrs) {
				if(std::get<0>(csr) == CSR_VL) {
					vl = std::get<1>(csr);
					break;
				}
			}
			return false;
		}
	}

	if((opc & 0x7F) == 0x57) {
		return true;
	}

	switch(opc) {
		case MATCH_VLB:
		case MATCH_VLXB:
		case MATCH_VLSB:
		case MATCH_VLH:
		case MATCH_VLXH:
		case MATCH_VLSH:
		case MATCH_VLW:
		case MATCH_VLXW:
		case MATCH_VLSW:
		case MATCH_VSB: 
		case MATCH_VSSB: 
		case MATCH_VSXB: 
		case MATCH_VSH: 
		case MATCH_VSSH: 
		case MATCH_VSXH: 
		case MATCH_VSW: 
		case MATCH_VSSW: 
		case MATCH_VSXW: return true;
	}

	return false;
}

void vector_tracer_t::trace(
	const working_set_t &ws, const insn_bits_t opc, const insn_t &insn) {

	vcount.inc();

	for(auto it : ws.diff.vregs) {
		uint8_t reg = std::get<0>(it);
		uint8_t pos = std::get<1>(it);
		uint32_t val = std::get<2>(it);
		vregs[reg][pos] = val;	
	}

	std::vector<bool> mask(vl, 0);
	for(uint16_t i = 0; i < vl; i++) {
		if(insn.vm()) mask[i] = vregs[0][i] & 0x1;
		else mask[i] = 0x1;
	}

	vinsn_stat_t *vinsn = new vinsn_stat_t();
	vinsn->opc = opc;
	vinsn->mask = mask;
	vinsn->src1 = -1;
	vinsn->src2 = -1;
	if(insn.vm()) {
		bool set_src1 = false;
		for(auto it : ws.input.vregs) {
			if(it != 0 && set_src1) {
				vinsn->src2 = (it & 0xf);
			} else if(it != 0) {
				vinsn->src1 = (it & 0xf);
				set_src1 = true;
			}
		}
	} else {
		bool set_src1 = false;
		for(auto it : ws.input.vregs) {
			if(set_src1) {
				vinsn->src2 = (it & 0xf);
				break;
			} else {
				vinsn->src1 = (it & 0xf);
				set_src1 = true;
			}
		}
	}

	vinsn->dest = *ws.output.vregs.begin();
	vinsn->vl = vl;
	vinsn->type = "ARITHMETIC";
	vinsn->repeat = -1;

	switch(opc) {
		case MATCH_VLB:
		case MATCH_VLXB:
		case MATCH_VLSB:
		case MATCH_VLH:
		case MATCH_VLXH:
		case MATCH_VLSH:
		case MATCH_VLW:
		case MATCH_VLXW:
		case MATCH_VLSW: {
			vinsn->type = "LOAD";

			std::set locs(ws.input.locs.begin(), ws.input.locs.end());

			bool has_updated = false;
			for(auto addr : locs) {
				if(updated.count(addr) != 0) {
					has_updated = true;
					thru_mem.inc();
				}
			}

			if(!has_updated) {
				uint32_t idx = loads.size();
				for(auto it = loads.rbegin(); it != loads.rend(); ++it, --idx) {
					bool match = true;
					if(*it == locs) {
						repeats.inc(vl);
						vinsn->repeat = vcount.get() - load_idxs[idx];
						break;
					}
				}
			}

			for(auto it : locs) updated.erase(it);

			loads.push_back(locs);
			load_idxs.push_back(vcount.get());
		}
	}

	switch(opc) {
		case MATCH_VSB: 
		case MATCH_VSSB: 
		case MATCH_VSXB: 
		case MATCH_VSH: 
		case MATCH_VSSH: 
		case MATCH_VSXH: 
		case MATCH_VSW: 
		case MATCH_VSSW: 
		case MATCH_VSXW: {
			vinsn->dest = -1;
			vinsn->type = "STORE";
			for(auto it : ws.output.locs) updated.insert(it);
		}
	}

	switch(opc) {
		MATCH_VREDOR:	
		MATCH_VREDSUM: vinsn->type = "REDUCTION";	
	}

	if(opc == MATCH_VPRESUM_V) vinsn->type = "PREFIX";

	if(opc == MATCH_VPERMUTE_V) vinsn->type = "PERMUTE";

	vinsns.push_back(vinsn);
}
