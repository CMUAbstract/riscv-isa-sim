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
	working_set_t(working_set_t *ws) {
		pc = ws->pc;
		input.regs = ws->input.regs;
		input.fregs = ws->input.fregs;
		input.vregs = ws->input.vregs;
		input.locs = ws->input.locs;
		input.csrs = ws->input.csrs;

		diff.regs = ws->diff.regs;
		diff.fregs = ws->diff.fregs;
		diff.vregs = ws->diff.vregs;
		diff.locs = ws->diff.locs;
		diff.csrs = ws->diff.csrs;

		output.regs = ws->output.regs;
		output.fregs = ws->output.fregs;
		output.vregs = ws->output.vregs;
		output.locs = ws->output.locs;
		output.csrs = ws->output.csrs;
	}
public:
	reg_t pc;
	struct {
		std::set<size_t> regs;
		std::set<size_t> fregs;
		std::set<size_t> vregs;
		std::set<addr_t> locs; 
		std::set<size_t> csrs;
	} input;
	struct {
		std::set<size_t> regs;
		std::set<size_t> fregs;
		std::set<size_t> vregs;
		std::set<addr_t> locs;
		std::set<size_t> csrs;
	} output;
	struct {
		std::vector<std::tuple<size_t, reg_t>> regs;
		std::vector<std::tuple<size_t, freg_t>> fregs;
		std::vector<std::tuple<size_t, size_t, reg_t>> vregs;
		std::vector<std::tuple<addr_t, uint8_t>> locs;
		std::vector<std::tuple<size_t, reg_t>> csrs;
	} diff;
public:
	size_t log_input_reg(size_t reg) {
		input.regs.insert(reg);
		return reg;
	}
	size_t log_input_freg(size_t reg) {
		input.fregs.insert(reg);
		return reg;
	}
	size_t log_input_vreg(size_t reg) {
		input.vregs.insert(reg);
		return reg;
	}
	size_t log_output_reg(size_t reg, reg_t value) {
		output.regs.insert(reg);
		diff.regs.push_back(std::make_tuple(reg, value));
		return reg;
	}
	size_t log_output_freg(size_t reg, freg_t value) {
		output.fregs.insert(reg);
		diff.fregs.push_back(std::make_tuple(reg, value));
		return reg;
	}
	size_t log_output_vreg(size_t reg, reg_t value, size_t pos) {
		output.vregs.insert(reg);
		diff.vregs.push_back(std::make_tuple(reg, pos, value));
		return reg;
	}
	size_t log_output_vreg(size_t reg, reg_t value[MAXVL]) {
		output.vregs.insert(reg);
		for(size_t i = 0; i < MAXVL; i++)
			diff.vregs.push_back(std::make_tuple(reg, i, value[i]));
		return reg;
	}
	addr_t log_input_loc(addr_t addr) {
		input.locs.insert(addr);
		return addr;
	}
	template <typename T>
	addr_t log_output_loc(addr_t addr, T value) {
		output.locs.insert(addr);
		uint8_t *bytes = (uint8_t *)&value;
		for(size_t i = 0; i < sizeof(T); i++)
			diff.locs.push_back(std::make_tuple(addr + i, bytes[i]));
		return addr;
	}
	size_t log_input_csr(size_t reg) {
		input.csrs.insert(reg);
		return reg;
	}
	size_t log_output_csr(size_t reg, reg_t value) {
		input.csrs.insert(reg);
		diff.csrs.push_back(std::make_tuple(reg, value));
		return reg;
	}
};

#endif