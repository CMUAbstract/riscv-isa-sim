#include "mem_tracer.h"

#include <cassert>

mem_tracer_t::~mem_tracer_t() {
	dump();
	for(auto it : transforms) {
		for(auto it2 : it.second) {
			delete it2;
		}
	}
}

bool mem_tracer_t::interested(
	const working_set_t &ws, const insn_bits_t opc, const insn_t &insn) {
	if(ws.pc != last_pc) {
		cycle++;
		last_pc = ws.pc;
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
		if(ws.input.locs.size()) return true;
		if(ws.output.locs.size()) return true;
		if(ws.input.vregs.size()) return true;
		if(ws.output.vregs.size()) return true;
	}
	return false;
}

void mem_tracer_t::account_vrf(
	const working_set_t &ws, std::vector<mem_tracer_t::access_stat_t*>& accesses) {
	for(auto it : ws.input.vregs) {
		for(uint32_t i = 0; i < vl; i++) {
			access_stat_t *access = new access_stat_t();
			access->pc = ws.pc & 0xFFFFFFFF;
			access->type = mem_tracer_t::access_stat_t::VRFR;
			access->addr = it;
			access->width = i;
			access->update = 0x0;
			accesses.push_back(access);
		}
	}
	for(auto it : ws.update.vregs) {
		access_stat_t *access = new access_stat_t();
		access->pc = ws.pc & 0xFFFFFFFF;
		access->type = mem_tracer_t::access_stat_t::VRFW;
		access->addr = std::get<0>(it);
		access->width = std::get<1>(it);
		access->update = std::get<2>(it);
		accesses.push_back(access);
	}
}

uint32_t reconstruct(addr_t addr, uint8_t width, 
	const std::vector<std::tuple<addr_t, uint8_t>>& locs) {
	uint32_t test = 0x1;
	uint8_t *endianness = (uint8_t*)&test;
	bool big_endian = (endianness[0] > 0) ? false : true;

	uint32_t val = 0x0;
	for(auto it : locs) {
		addr_t byte_addr = std::get<0>(it) & 0xFFFFFFFF;
		uint32_t byte_val = static_cast<uint32_t>(std::get<1>(it));
		if(byte_addr < addr + width && byte_addr >= addr) {
			uint32_t byte_offset = byte_addr - addr;
			if(big_endian) {
				byte_offset = 0x3 - byte_offset;
			}
			assert(byte_offset < width);
			val |= byte_val << (byte_offset * 0x8);
		}
	}
	// Check for sign and extend if needed
	if((val >> (width * 0x8 - 1)) > 0x0) {
		for(uint8_t i = 0; i < sizeof(uint32_t) - width; i++) {
			val |= 0xFF << ((width + i) * 0x8); 
		}
	}
	return val;
}

void mem_tracer_t::trace(
	const working_set_t &ws, const insn_bits_t opc, const insn_t &insn) {
	std::vector<access_stat_t *> accesses;
	// Vector load
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
			for(auto it : ws.input.locs) {
				access_stat_t *access = new access_stat_t();
				access->pc = ws.pc & 0xFFFFFFFF;
				access->type = mem_tracer_t::access_stat_t::VLOAD;
				access->addr = it & 0xFFFFFFFF;
				access->update = 0x0;
				switch((insn.bits() & 0x7000) >> 0xC) {
					case 0x0: access->width = 0x1; break;
					case 0x5: access->width = 0x2; break;
					case 0x6: access->width = 0x4; break;
					default: access->width = 0x4; break;
				}
				if(it % access->width == 0) {
					accesses.push_back(access);
				} else {
					delete access;
				}
			}
			account_vrf(ws, accesses);
			transforms.insert(cycle, accesses);
			return;
		}
	}

	// Vector store
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
			for(auto it : ws.output.locs) {
				access_stat_t *access = new access_stat_t();
				access->pc = ws.pc & 0xFFFFFFFF;
				access->type = mem_tracer_t::access_stat_t::VSTORE;
				access->addr = it & 0xFFFFFFFF;
				switch((insn.bits() & 0x7000) >> 0xC) {
					case 0x0: access->width = 0x1; break;
					case 0x5: access->width = 0x2; break;
					case 0x6: access->width = 0x4; break;
					default: access->width = 0x4; break;
				}
				access->update = reconstruct(
					access->addr, access->width, ws.update.locs);
				if(access->addr % access->width == 0) {
					accesses.push_back(access);
				} else {
					delete access;
				}
			}
			account_vrf(ws, accesses);
			transforms.insert(cycle, accesses);
			return;
		}
	}	

	// Load
	if(ws.input.locs.size()) {
		access_stat_t *access = new access_stat_t();
		access->pc = ws.pc & 0xFFFFFFFF;
		access->type = mem_tracer_t::access_stat_t::LOAD;
		access->addr = *ws.input.locs.begin() & 0xFFFFFFFF;
		access->update = 0x0;
		switch(opc) {
			case MATCH_LB: access->width = 0x1; break;
			case MATCH_LBU: access->width = 0x1; break;
			case MATCH_LH: access->width = 0x2; break;
			case MATCH_LHU: access->width = 0x2; break;
			case MATCH_LW: access->width = 0x4; break;
			case MATCH_LWU: access->width = 0x4; break;
			default: access->width = 0x4; break; 
		}
		accesses.push_back(access);
		transforms.insert(cycle, accesses);
		return;
	}

	// Store
	if(ws.output.locs.size()) {
		access_stat_t *access = new access_stat_t();
		access->pc = ws.pc & 0xFFFFFFFF;
		access->type = mem_tracer_t::access_stat_t::STORE;
		access->addr = *ws.output.locs.begin() & 0xFFFFFFFF;
		switch(opc) {
			case MATCH_SB: access->width = 0x1; break;
			case MATCH_SH: access->width = 0x2; break;
			case MATCH_SW: access->width = 0x4; break;
			default: access->width = 0x4; break; 
		}
		access->update = reconstruct(access->addr, access->width, ws.update.locs);
		accesses.push_back(access);
		transforms.insert(cycle, accesses);
		return;
	}

	// Normal vector operation
	if(ws.input.vregs.size() || ws.output.vregs.size()) {
		account_vrf(ws, accesses);
		transforms.insert(cycle, accesses);
		return;
	}
}

io::json mem_tracer_t::access_stat_t::to_json() const {
	std::string type_str;
	switch(type) {
		case VRFR: type_str = "VRFR"; break;
		case VRFW: type_str = "VRFW"; break;
		case VLOAD: type_str = "VLOAD"; break;
		case VSTORE: type_str = "VSTORE"; break;
		case STORE: type_str = "STORE"; break;
		case LOAD: type_str = "LOAD"; break;
		default: break;
	}

	return io::json::object {
		{"type", type_str},
		{"addr", addr},
		{"pc", pc},
		{"width", width},
		{"update", update}
	};
}