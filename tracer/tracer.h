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
#include <hstd/memory.h>

#include "working_set.h"

class tracer_t: public io::serializable {
public:
	tracer_t() {}
	virtual ~tracer_t() {}
	virtual bool interested(
		const working_set_t &ws, const insn_bits_t opc, const insn_t &insn) = 0;
	virtual void trace(
		const working_set_t &ws, const insn_bits_t opc, const insn_t &insn) = 0;
	virtual void tabulate() = 0;
	virtual void reset(reset_level_t level, uint32_t minstret) {}
	virtual void reset(reset_level_t level=HARD) {}
	virtual void set_outdir(const std::string &_outdir) { outdir = _outdir; }
	virtual void set_hyperdrive(bool _hyperdrive=true) { hyperdrive = _hyperdrive; }
	virtual void set_intermittent(bool _intermittent=true) {}
	virtual void set_roi(addr_t start_pc, addr_t end_pc) { 
		roi_start = start_pc;
		roi_end = end_pc;
	}
protected:
	std::string outdir;
	bool hyperdrive = false;
	addr_t roi_start = 0;
	addr_t roi_end = 0;
};

class elfloader_t;
class tracer_impl_t : public tracer_t {
public:
	tracer_impl_t(std::string _name, io::json _config, elfloader_t *_elf);
	virtual io::json to_json() const;
	virtual void dump();
protected:
	std::string name;
	io::json config;
	elfloader_t *elf;
};

class tracer_list_t : public tracer_impl_t {
public:
	tracer_list_t(io::json _config, elfloader_t *_elf)
		: tracer_impl_t("tracer_list", _config, _elf) {}
	~tracer_list_t() {
		for(auto it : list) delete it;
	}
	bool interested(
		const working_set_t &ws, const insn_bits_t opc, const insn_t &insn);
	virtual void trace(
		const working_set_t &ws, const insn_bits_t opc, const insn_t &insn) {
		for(auto it : list) it->trace(ws, opc, insn);
	}
	void tabulate() {
		for(auto it : list) it->tabulate();
	}
	io::json to_json() const {
		return io::json(list);
	}
	void reset(reset_level_t level, uint32_t minstret) {
		for(auto it : list) it->reset(level, minstret);
	}
	void reset(reset_level_t level=HARD) {
		for(auto it : list) it->reset(level);
	}
	virtual void set_outdir(const std::string &_outdir) {
		tracer_t::set_outdir(_outdir);
		for(auto it : list) it->set_outdir(_outdir);
	}
	virtual void set_hyperdrive(bool _hyperdrive=true) { 
		tracer_t::set_hyperdrive(_hyperdrive);
		for(auto it : list) it->set_hyperdrive(_hyperdrive);
	}
	virtual void set_intermittent(bool _intermittent=true) { 
		tracer_t::set_intermittent(_intermittent);
		for(auto it : list) it->set_intermittent(_intermittent);
	}
	virtual void set_roi(addr_t start_pc, addr_t end_pc) {
		tracer_t::set_roi(start_pc, end_pc);
		for(auto it : list) it->set_roi(start_pc, end_pc);
	}
	void push_back(tracer_t *t) { list.push_back(t); }
	std::vector<tracer_t *>::iterator begin() { return list.begin(); }
	std::vector<tracer_t *>::iterator end() { return list.end(); }
protected:
	std::vector<tracer_t *> list;
};

#define DIFF_LIST_SIZE 0x100

// TODO: custom reverse iterator
class core_tracer_t: public tracer_list_t {
public:
	typedef std::vector<hstd::shared_ptr<working_set_t>> vec_ws_t;
	class diff_list_t {
	public:
		diff_list_t() {}
		diff_list_t(diff_list_t &d, uint32_t offset); 
		void push_back(hstd::shared_ptr<working_set_t> ws);
		uint32_t size(void) const { return diffs.size(); }
		uint32_t head(void) const { return head_ptr; }
		hstd::shared_ptr<working_set_t> get(uint32_t idx) { return diffs[idx]; }
		vec_ws_t::reverse_iterator rbegin() { return diffs.rbegin(); }
		vec_ws_t::reverse_iterator rend() { return diffs.rend(); }
	private:
		vec_ws_t diffs;
		uint32_t head_ptr = 0;
	};
public:
	core_tracer_t(std::string _config, elfloader_t *_elf);
	~core_tracer_t() {}
	void trace(const working_set_t &ws, insn_bits_t opc, insn_t insn){
		diffs.push_back(hstd::shared_ptr<working_set_t>(new working_set_t(ws)));
		tracer_list_t::trace(ws, opc, insn);
	}
	hstd::shared_ptr<diff_list_t> get_diff(uint32_t minstret_delta) {
		return hstd::shared_ptr<diff_list_t>(
			new diff_list_t(diffs, diffs.size() - minstret_delta));
	}
private:
	void init();
	diff_list_t diffs;
};

#endif
