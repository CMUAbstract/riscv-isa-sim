#ifndef FETCH_H
#define FETCH_H

#include <hstd/memory.h>
#include <vector>

#include "port.h"
#include "module.h"
#include "insn_info.h"

class insn_fetch_value_t;
class insn_decode_value_t;
class insn_squash_value_t;
class mem_read_value_t;
class mem_retire_value_t;
class fetch_t : public module_t {
public:
	fetch_t(std::string _name, io::json _config, scheduler_t *_scheduler);
	void exec();
private:
	bool occupied = false;
	size_t idx = 0;
private:
	port_t<bool> *bp_port;
	port_t<insn_fetch_value_t> *insn_fetch_port;
	port_t<insn_decode_value_t> *insn_decode_port;
	port_t<insn_squash_value_t> *insn_squash_port;
	port_t<mem_read_value_t> *mem_read_port;
	port_t<mem_retire_value_t> *mem_retire_port;
};

#endif