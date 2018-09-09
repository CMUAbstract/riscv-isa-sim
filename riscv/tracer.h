// See LICENSE for license details.

#ifndef _RISCV_TRACER_H
#define _RISCV_TRACER_H

#include <set>
#include <map>

#include <fesvr/memif.h>

#include "processor.h"
#include "stat.h"

struct working_set_t {
	struct {
		std::set<size_t> regs;
		std::set<size_t> fregs;
		std::set<addr_t> locs; 
	} input;
	struct {
		std::set<size_t> regs;
		std::set<size_t> fregs;
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
	size_t log_output_reg(size_t reg) {
		output.regs.insert(reg);
		return reg;
	}
	size_t log_output_freg(size_t reg) {
		output.fregs.insert(reg);
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

class tracer_t {
public:
	tracer_t() {}
	virtual ~tracer_t() {}
	virtual bool interested(
		processor_t *p, insn_bits_t opc, insn_t insn, working_set_t ws) = 0;
	virtual void trace(
		processor_t *p, insn_bits_t opc, insn_t insn, working_set_t ws) = 0;
};

class tracer_list_t : public tracer_t {
public:
	bool interested(
		processor_t *p, insn_bits_t opc, insn_t insn, working_set_t ws) {
		for (auto it = list.begin(); it != list.end(); ++it)
			if ((*it)->interested(p, opc, insn, ws))
				return true;
		return false;
	}
	void trace(processor_t *p, insn_bits_t opc, insn_t insn, working_set_t ws){
		for (auto it = list.begin(); it != list.end(); ++it)
			(*it)->trace(p, opc, insn, ws);
	}
	void push_back(tracer_t *t) { list.push_back(t); }
	std::vector<tracer_t *>::iterator begin() { return list.begin(); }
	std::vector<tracer_t *>::iterator end() { return list.end(); }
private:
	std::vector<tracer_t *> list;
}; 

// Tracks reads and writes to locations in memory
class basic_mem_tracer_t : public tracer_t {
public:
	bool interested(
		processor_t *p, insn_bits_t opc, insn_t insn, working_set_t ws) {
		return true;
	}
	void trace(processor_t *p, insn_bits_t opc, insn_t insn, working_set_t ws);
	~basic_mem_tracer_t();
private:
	class mem_loc_stat_t : public stat_t {
	public:
		mem_loc_stat_t() : mem_loc_stat_t("", "") {
			reads.reset();	
			writes.reset();
		}
		mem_loc_stat_t(std::string _name, std::string _desc) :
			stat_t(_name, _desc), reads("reads", ""), writes("writes", "") {}
		std::string dump(void) const {
			std::ostringstream os;
			os << "{" << reads.dump() << ",";
			os << writes.dump() << "}";
			return os.str();
		}
		counter_stat_t<size_t> reads;
		counter_stat_t<size_t> writes;
	};
	map_stat_t<addr_t, mem_loc_stat_t *> tracked_locations;
};

// Generate Miss curves
class miss_curve_tracer_t : public tracer_t {
public:
	~miss_curve_tracer_t();
	bool interested(
		processor_t *p, insn_bits_t opc, insn_t insn, working_set_t ws) {
		return true;
	}
	void trace(processor_t *p, insn_bits_t opc, insn_t insn, working_set_t ws);
private:
	std::map<addr_t, reg_t> tracked_locations;
	map_stat_t<size_t, counter_stat_t<size_t> *> histogram;
	reg_t minstret = 0;
};

#endif
