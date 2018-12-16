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
		std::set<addr_t> locs; 
		std::set<uint32_t> csrs;
	} input;
	struct {
		std::set<uint32_t> regs;
		std::set<uint32_t> fregs;
		std::set<uint32_t> vregs;
		std::set<addr_t> locs;
		std::set<std::tuple<uint32_t, reg_t>> csrs;
	} output;
	struct {
		std::vector<std::tuple<uint32_t, reg_t>> regs;
		std::vector<std::tuple<uint32_t, freg_t>> fregs;
		std::vector<std::tuple<uint32_t, uint32_t, reg_t>> vregs;
		std::vector<std::tuple<addr_t, uint8_t>> locs;
		std::vector<std::tuple<uint32_t, reg_t>> csrs;
	} diff;
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
	uint32_t log_output_reg(uint32_t reg, reg_t value) {
		output.regs.insert(reg);
		diff.regs.push_back(std::make_tuple(reg, value));
		return reg;
	}
	uint32_t log_output_freg(uint32_t reg, freg_t value) {
		output.fregs.insert(reg);
		diff.fregs.push_back(std::make_tuple(reg, value));
		return reg;
	}
	uint32_t log_output_vreg(uint32_t reg, reg_t value, uint32_t pos) {
		output.vregs.insert(reg);
		diff.vregs.push_back(std::make_tuple(reg, pos, value));
		return reg;
	}
	uint32_t log_output_vreg(uint32_t reg, reg_t value[MAXVL]) {
		output.vregs.insert(reg);
		for(uint32_t i = 0; i < MAXVL; i++)
			diff.vregs.push_back(std::make_tuple(reg, i, value[i]));
		return reg;
	}
	template <typename T>
	addr_t log_input_loc(addr_t addr) {
		input.locs.insert(addr);
		if(sizeof(T) == 8) input.locs.insert(addr + sizeof(uint32_t));
		return addr;
	}
	template <typename T>
	addr_t log_output_loc(addr_t addr, T value) {
		output.locs.insert(addr);
		if(sizeof(T) == 8) output.locs.insert(addr + sizeof(uint32_t));
		uint8_t *bytes = (uint8_t *)&value;
		for(uint32_t i = 0; i < sizeof(T); i++) {
			diff.locs.push_back(std::make_tuple(addr + i, bytes[i]));
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
		return reg;
	}
	addr_t log_next_pc(addr_t addr) {
		next_pc = addr;
		return addr;
	}
};

#endif