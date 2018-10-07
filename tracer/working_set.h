#ifndef WORKING_SET_H
#define WORKING_SET_H

#include <set>

#include <fesvr/memif.h>

struct working_set_t {
	struct {
		std::set<size_t> regs;
		std::set<size_t> fregs;
		std::set<size_t> vregs;
		std::set<addr_t> locs; 
	} input;
	struct {
		std::set<size_t> regs;
		std::set<size_t> fregs;
		std::set<size_t> vregs;
		std::set<addr_t> locs; 
	} output;
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
	size_t log_output_reg(size_t reg) {
		output.regs.insert(reg);
		return reg;
	}
	size_t log_output_freg(size_t reg) {
		output.fregs.insert(reg);
		return reg;
	}
	size_t log_output_vreg(size_t reg) {
		output.vregs.insert(reg);
		return reg;
	}
	addr_t log_input_loc(addr_t addr) {
		input.locs.insert(addr);
		return addr;
	}
	addr_t log_output_loc(addr_t addr) {
		output.locs.insert(addr);
		return addr;
	}
};

#endif