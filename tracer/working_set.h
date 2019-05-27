#ifndef WORKING_SET_H
#define WORKING_SET_H

#include <set>
#include <vector>
#include <tuple>

#include <fesvr/memif.h>
#include <common/decode.h>

struct working_set_t {
public:
	working_set_t() {}
	working_set_t(const working_set_t &ws) {
		pc = ws.pc;
		next_pc = ws.next_pc;
		input.regs = ws.input.regs;
		input.fregs = ws.input.fregs;
		input.vregs = ws.input.vregs;
		input.locs = ws.input.locs;
		input.csrs = ws.input.csrs;

		diff.regs = ws.diff.regs;
		diff.fregs = ws.diff.fregs;
		diff.vregs = ws.diff.vregs;
		diff.locs = ws.diff.locs;
		diff.csrs = ws.diff.csrs;

		output.regs = ws.output.regs;
		output.fregs = ws.output.fregs;
		output.vregs = ws.output.vregs;
		output.locs = ws.output.locs;
		output.csrs = ws.output.csrs;
	}
public:
	reg_t pc = 0;
	reg_t next_pc = 0;
	struct {
		std::set<uint32_t> regs;
		std::set<uint32_t> fregs;
		std::set<uint32_t> vregs;
		std::vector<addr_t> locs; 
		std::set<uint32_t> csrs;
	} input;
	struct {
		std::set<uint32_t> regs;
		std::set<uint32_t> fregs;
		std::set<uint32_t> vregs;
		std::vector<addr_t> locs;
		std::set<std::tuple<uint32_t, reg_t>> csrs;
	} output;
	struct {
		std::vector<std::tuple<uint32_t, reg_t>> regs;
		std::vector<std::tuple<uint32_t, freg_t>> fregs;
		std::vector<std::tuple<uint32_t, uint32_t, reg_t>> vregs;
		std::vector<std::tuple<addr_t, uint8_t>> locs;
		std::vector<std::tuple<uint32_t, reg_t>> csrs;
	} diff;
	struct {
		std::vector<std::tuple<uint32_t, reg_t>> regs;
		std::vector<std::tuple<uint32_t, freg_t>> fregs;
		std::vector<std::tuple<uint32_t, uint32_t, reg_t>> vregs;
		std::vector<std::tuple<addr_t, uint8_t>> locs;
		std::vector<std::tuple<uint32_t, reg_t>> csrs;
	} update;
public:
	uint32_t log_input_reg(uint32_t reg) {
		input.regs.insert(reg);
		return reg;
	}
	uint32_t log_input_freg(uint32_t reg) {
		input.fregs.insert(reg);
		return reg;
	}
	uint32_t log_input_vreg(uint32_t reg) {
		input.vregs.insert(reg);
		return reg;
	}
	uint32_t log_output_reg(uint32_t reg, reg_t old_value, reg_t new_value) {
		output.regs.insert(reg);
		update.regs.push_back(std::make_tuple(reg, new_value));
		diff.regs.push_back(std::make_tuple(reg, old_value));
		return reg;
	}
	uint32_t log_output_freg(uint32_t reg, freg_t old_value, freg_t new_value) {
		output.fregs.insert(reg);
		update.fregs.push_back(std::make_tuple(reg, new_value));
		diff.fregs.push_back(std::make_tuple(reg, old_value));
		return reg;
	}
	uint32_t log_output_vreg(uint32_t reg, reg_t old_value, reg_t new_value, uint32_t pos) {
		output.vregs.insert(reg);
		update.vregs.push_back(std::make_tuple(reg, pos, new_value));
		diff.vregs.push_back(std::make_tuple(reg, pos, old_value));
		return reg;
	}
	uint32_t log_output_vreg(uint32_t reg, std::vector<reg_t>& old_value, 
		std::vector<reg_t>& new_value) {
		output.vregs.insert(reg);
		for(uint32_t i = 0; i < max_vl; i++) {
			update.vregs.push_back(std::make_tuple(reg, i, new_value[i]));
			diff.vregs.push_back(std::make_tuple(reg, i, old_value[i]));
		}
		return reg;
	}
	template <typename T>
	addr_t log_input_loc(addr_t addr) {
		input.locs.push_back(addr);
		if(sizeof(T) == 8) input.locs.push_back(addr + sizeof(uint32_t));
		return addr;
	}
	template <typename T>
	addr_t log_output_loc(addr_t addr, T old_value, T new_value) {
		output.locs.push_back(addr);
		if(sizeof(T) == 8) output.locs.push_back(addr + sizeof(uint32_t));
		uint8_t *old_bytes = (uint8_t *)&old_value;
		uint8_t *new_bytes = (uint8_t *)&new_value;
		for(uint32_t i = 0; i < sizeof(T); i++) {
			update.locs.push_back(std::make_tuple(addr + i, new_bytes[i]));
			diff.locs.push_back(std::make_tuple(addr + i, old_bytes[i]));
		}
		return addr;
	}
	uint32_t log_input_csr(uint32_t reg) {
		input.csrs.insert(reg);
		return reg;
	}
	uint32_t log_output_csr(uint32_t reg, reg_t old_value, reg_t new_value) {
		output.csrs.insert(std::make_tuple(reg, new_value));
		diff.csrs.push_back(std::make_tuple(reg, old_value));
		update.csrs.push_back(std::make_tuple(reg, new_value));
		return reg;
	}
	addr_t log_next_pc(addr_t addr) {
		next_pc = addr;
		return addr;
	}
};

#endif