// See LICENSE for license details.

#ifndef TRACER_H
#define TRACER_H

#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include <fesvr/memif.h>
#include <common/decode.h>
#include <io/io.h>

class working_set_t;
class tracer_t {
public:
	tracer_t() {}
	virtual ~tracer_t() {}
	virtual bool interested(working_set_t *ws, insn_bits_t opc, insn_t insn) = 0;
	virtual void trace(working_set_t *ws, insn_bits_t opc, insn_t insn) = 0;
	virtual void tabulate() = 0;
	virtual std::string dump() = 0;
};

class tracer_list_t : public tracer_t {
public:
	bool interested(working_set_t *ws, insn_bits_t opc, insn_t insn) {
		for (auto it = list.begin(); it != list.end(); ++it)
			if ((*it)->interested(ws, opc, insn))
				return true;
		return false;
	}
	void trace(working_set_t *ws, insn_bits_t opc, insn_t insn){
		for (auto it = list.begin(); it != list.end(); ++it)
			(*it)->trace(ws, opc, insn);
	}
	void tabulate() {
		for (auto it = list.begin(); it != list.end(); ++it)
			(*it)->tabulate();
	}
	std::string dump() {
		std::string str;
		for (auto it = list.begin(); it != list.end(); ++it)
			str += (*it)->dump();
		return str;
	}
	void push_back(tracer_t *t) { list.push_back(t); }
	std::vector<tracer_t *>::iterator begin() { return list.begin(); }
	std::vector<tracer_t *>::iterator end() { return list.end(); }
private:
	std::vector<tracer_t *> list;
};

class elfloader_t;
class tracer_impl_t : public tracer_t{
public:
	tracer_impl_t(io::json _config, elfloader_t *_elf) 
		: tracer_t(), config(_config), elf(_elf) {}
protected:
	io::json config;
	elfloader_t *elf;
};

class core_tracer_t: public tracer_impl_t {
public:
	core_tracer_t(std::string _config, elfloader_t *_elf);
	~core_tracer_t();
	bool interested(working_set_t *ws, insn_bits_t opc, insn_t insn) {
		return tracers.interested(ws, opc, insn);
	}
	void trace(working_set_t *ws, insn_bits_t opc, insn_t insn) {
		tracers.trace(ws, opc, insn);
	}
	void tabulate() { tracers.tabulate(); }
	virtual std::string dump();
private:
	void init();
private:
	tracer_list_t tracers;
};

/*class printer_t {
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
}; */

#endif
