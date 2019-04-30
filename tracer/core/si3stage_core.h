#ifndef SINGLE_ISSUE_3_STAGE_H
#define SINGLE_ISSUE_3_STAGE_H

#include <map>
#include <sstream>

#include <fesvr/memif.h>
#include <stat/stat.h>

#include "value.h"
#include "core.h"

class si3stage_core_t : core_t {
public:
	friend class vcu_t; 
	si3stage_core_t(std::string _name, io::json _config, value_heap_t *_values);
	
	io::json to_json() const;
	void reset();

	void buffer_insn(hstd::shared_ptr<insn_info_t> insn);
	void next_insn();
public:
	class fetch_t : public module_t {
	public:
		fetch_t(std::string _name, io::json _config, scheduler_t *_scheduler);
	private:
		bool occupied = false;
		hstd::shared_pointer<insn_info_t *> cur_insn;
	private:
		port_t<bool> *bp_port;
		port_t<insn_fetch_value_t> *insn_fetch_port;
		port_t<insn_decode_value_t> *insn_decode_port;
		port_t<mem_read_value_t> *mem_read_port;
		port_t<mem_retire_value_t> *mem_retire_port;
	};

private:
	port_t<mem_read_value_t> icache_read_port;
	port_t<mem_read_value_t> icache_retire_port;
	port_t<mem_read_value_t> mem_read_port;
	port_t<mem_write_value_t> mem_write_port;
	port_t<mem_ready_value_t> mem_ready_port;
	port_t<mem_retire_value_t> mem_retire_port;

private:
	branch_predictor_t *predictor;
private:
	counter_stat_t<uint32_t> squashes;
	counter_stat_t<uint32_t> flushes;
	counter_stat_t<uint32_t> jumps;
	counter_stat_t<uint32_t> branches;
};

#endif