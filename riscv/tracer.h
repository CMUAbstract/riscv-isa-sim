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
class elfloader_t;
class basic_mem_tracer_t : public tracer_t {
public:
	basic_mem_tracer_t(elfloader_t *_elf) : tracer_t(), elf(_elf) {}
	~basic_mem_tracer_t();
	bool interested(
		processor_t *p, insn_bits_t opc, insn_t insn, working_set_t ws) {
		return true;
	}
	void trace(processor_t *p, insn_bits_t opc, insn_t insn, working_set_t ws);
private:
	class mem_loc_stat_t : public stat_t {
	public:
		mem_loc_stat_t() : mem_loc_stat_t("", "") {}
		mem_loc_stat_t(std::string _name, std::string _desc) :
			stat_t(_name, _desc), reads("reads", ""), writes("writes", "") {
			reads.reset();	
			writes.reset();
		}
		std::string dump(void) const;
		std::string symbol;
		std::string section;
		counter_stat_t<size_t> reads;
		counter_stat_t<size_t> writes;
	};
	map_stat_t<addr_t, mem_loc_stat_t *> tracked_locations;
	struct region_t {
		addr_t base;
		size_t size;
		region_t() : region_t(0, 0) {}
		region_t(addr_t _base, size_t _size) : base(_base), size(_size) {}
		region_t(const region_t &other) {base = other.base; size = other.size; } 
	};
	elfloader_t *elf;
};

// Generate Miss curves
class miss_curve_tracer_t : public tracer_t {
public:
	miss_curve_tracer_t(elfloader_t *_elf);
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
	addr_t text_base = 0;
	size_t text_size = 0;
	elfloader_t *elf;
};

#endif
