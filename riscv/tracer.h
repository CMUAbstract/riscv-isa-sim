// See LICENSE for license details.

#ifndef _RISCV_TRACER_H
#define _RISCV_TRACER_H

#include <set>
#include <map>
#include <iostream>
#include <fstream>

#include <fesvr/memif.h>

#include "processor.h"
#include "stat.h"

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

class printer_t {
public:
	printer_t(std::string outdir, std::string fn) {
		dealloc = true;
		os = new std::ofstream(outdir + "/" + fn); 
	}
	printer_t() { os = &std::cerr; }
	~printer_t() { if(dealloc) delete os; }
protected:
	std::ostream *os;
	bool dealloc = false;
};

class elfloader_t;
class tracer_impl_t : public tracer_t, public printer_t {
public:
	tracer_impl_t(elfloader_t *_elf) : tracer_t(), printer_t(), elf(_elf) {}
	tracer_impl_t(elfloader_t *_elf, std::string path, std::string fn) : 
		tracer_t(), printer_t(path, fn), elf(_elf) {}
protected:
	elfloader_t *elf;
};

// Tracks reads and writes to locations in memory
class basic_mem_tracer_t : public tracer_impl_t {
public:
	basic_mem_tracer_t(elfloader_t *_elf) : tracer_impl_t(_elf) {}
	basic_mem_tracer_t(elfloader_t *_elf, std::string outdir, std::string fn = "mem.json") :
		tracer_impl_t(_elf, outdir, fn) {}
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
};

// Generate Miss curves
class insn_curve_tracer_t : public tracer_impl_t {
public:
	insn_curve_tracer_t(elfloader_t *_elf, std::string outdir, std::string fn = "insn.json") :
		tracer_impl_t(_elf, outdir, fn) {
		init();
	}
	insn_curve_tracer_t(elfloader_t *_elf) : tracer_impl_t(_elf) {
		init();
	}
	~insn_curve_tracer_t();
	bool interested(
		processor_t *p, insn_bits_t opc, insn_t insn, working_set_t ws) {
		return true;
	}
	void trace(processor_t *p, insn_bits_t opc, insn_t insn, working_set_t ws);
protected:
	void init(void);
	std::map<addr_t, reg_t> tracked_locations;
	map_stat_t<size_t, counter_stat_t<size_t> *> histogram;
	addr_t text_base = 0;
	size_t text_size = 0;
private:
	reg_t minstret = 0;
};

class miss_curve_tracer_t : public insn_curve_tracer_t {
public:
	miss_curve_tracer_t(elfloader_t *_elf) : insn_curve_tracer_t(_elf) {}
	miss_curve_tracer_t(elfloader_t *_elf, std::string outdir, std::string fn = "miss.json") :
		insn_curve_tracer_t(_elf, outdir, fn) {}
	void trace(processor_t *p, insn_bits_t opc, insn_t insn, working_set_t ws);
private:
	reg_t maccess = 0;
};

class insn_tracer_t : public tracer_impl_t {
public:
	insn_tracer_t(elfloader_t *_elf) : tracer_impl_t(_elf) {
		if(!insn_registered) register_insn_types();
		insn_registered = true;
	}
	insn_tracer_t(elfloader_t *_elf, std::string outdir, std::string fn) :
		tracer_impl_t(_elf, outdir, fn) {
		if(!insn_registered) register_insn_types();
		insn_registered = true;
	}
	~insn_tracer_t() {}
protected:
	static std::map<insn_bits_t, uint64_t> m;
private:
	void register_insn_types(void);
	static bool insn_registered;
};

class perf_tracer_t : public insn_tracer_t {
public:
	perf_tracer_t(elfloader_t *_elf) : insn_tracer_t(_elf), mcycles("cycles", "") {}
	perf_tracer_t(elfloader_t *_elf, std::string outdir, std::string fn = "perf.json") :
		insn_tracer_t(_elf, outdir, fn), mcycles("cycles", "") {}
	~perf_tracer_t();
	bool interested(
		processor_t *p, insn_bits_t opc, insn_t insn, working_set_t ws) {
		return true;
	}
	void trace(processor_t *p, insn_bits_t opc, insn_t insn, working_set_t ws);
private:
	counter_stat_t<uint64_t> mcycles;
};

class energy_tracer_t : public insn_tracer_t {
public:
	energy_tracer_t(elfloader_t *_elf) : insn_tracer_t(_elf), menergy("energy", "") {
		menergy.reset();
	}
	energy_tracer_t(elfloader_t *_elf, std::string outdir, std::string fn = "energy.json") :
		insn_tracer_t(_elf, outdir, fn), menergy("energy", "") {
		menergy.reset();
	}
	bool interested(
		processor_t *p, insn_bits_t opc, insn_t insn, working_set_t ws) {
		return true;
	}
	~energy_tracer_t();
	void trace(processor_t *p, insn_bits_t opc, insn_t insn, working_set_t ws);
private:
	counter_stat_t<float> menergy; 
};

#endif
