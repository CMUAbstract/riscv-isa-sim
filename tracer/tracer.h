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

#include "working_set.h"
#include "smartptr.h"

class tracer_t {
public:
	tracer_t() {}
	virtual ~tracer_t() {}
	virtual bool interested(working_set_t *ws, insn_bits_t opc, insn_t insn) = 0;
	virtual void trace(working_set_t *ws, insn_bits_t opc, insn_t insn) = 0;
	virtual void tabulate() = 0;
	virtual void reset(size_t minstret) {}
	virtual size_t minstret() { return 0; }
	virtual std::string dump() = 0;
};

class elfloader_t;
class tracer_impl_t : public tracer_t {
public:
	tracer_impl_t(io::json _config, elfloader_t *_elf) 
		: tracer_t(), config(_config), elf(_elf) {}
protected:
	io::json config;
	elfloader_t *elf;
};

class tracer_list_t : public tracer_impl_t {
public:
	tracer_list_t(io::json _config, elfloader_t *_elf)
		: tracer_impl_t(_config, _elf) {}
	bool interested(working_set_t *ws, insn_bits_t opc, insn_t insn) {
		for(auto it : list)
			if (it->interested(ws, opc, insn))
				return true;
		return false;
	}
	virtual void trace(working_set_t *ws, insn_bits_t opc, insn_t insn){
		for(auto it : list) it->trace(ws, opc, insn);
	}
	void tabulate() {
		for(auto it : list) it->tabulate();
	}
	void reset(size_t minstret) {
		for(auto it : list) it->reset(minstret);
	}
	size_t minstret() {
		size_t min = list[0]->minstret();
		for(auto it : list) {
			size_t tmp = it->minstret();
			if(tmp < min) min = tmp;
		}
		return min;
	}
	std::string dump() {
		std::string str;
		for(auto it : list) str += it->dump();
		return str;
	}
	void push_back(tracer_t *t) { list.push_back(t); }
	std::vector<tracer_t *>::iterator begin() { return list.begin(); }
	std::vector<tracer_t *>::iterator end() { return list.end(); }
protected:
	std::vector<tracer_t *> list;
};

class core_tracer_t: public tracer_list_t {
public:
	typedef std::vector<shared_ptr_t<working_set_t>> vec_ws_t;
	class diff_list_t {
	public:
		diff_list_t() {}
		diff_list_t(const diff_list_t &d, size_t offset) 
			: diffs(d.begin() + offset, d.end()) {}
		void push_back(shared_ptr_t<working_set_t> ws) { diffs.push_back(ws); }
		size_t size(void) const { return diffs.size(); }
		vec_ws_t::iterator begin() { return diffs.begin(); }
		vec_ws_t::const_iterator begin() const { return diffs.begin(); }
		vec_ws_t::reverse_iterator rbegin() { return diffs.rbegin(); }
		vec_ws_t::iterator end() { return diffs.end(); }
		vec_ws_t::const_iterator end() const { return diffs.end(); }
		vec_ws_t::reverse_iterator rend() { return diffs.rend(); }
	private:
		std::vector<shared_ptr_t<working_set_t>> diffs;
	};
public:
	core_tracer_t(std::string _config, elfloader_t *_elf);
	~core_tracer_t() {}
	void trace(working_set_t *ws, insn_bits_t opc, insn_t insn){
		diffs.push_back(shared_ptr_t<working_set_t>(new working_set_t(ws)));
		tracer_list_t::trace(ws, opc, insn);
	}
	shared_ptr_t<diff_list_t> get_diff(size_t minstret_delta) {
		return shared_ptr_t<diff_list_t>(
			new diff_list_t(diffs, diffs.size() - minstret_delta));
	}
private:
	void init();
	diff_list_t diffs;
};

#endif
