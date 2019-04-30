#ifndef EXEC_H
#define EXEC_H

#include <hstd/memory.h>

#include "port.h"
#include "module.h"
#include "insn_info.h"

class insn_exec_value_t;
class insn_retire_value_t;
class insn_squash_value_t;
class mem_read_value_t;
class mem_write_value_t;
class mem_retire_value_t;
class mem_ready_value_t;
class reg_write_value_t;
class exec_t : public module_t {
public:
	exec_t(std::string _name, io::json _config, scheduler_t *_scheduler);
private:
	bool occupied = false;
private:
	port_t<bool> *bp_port;
	port_t<insn_exec_value_t> *insn_exec_port;
	port_t<insn_retire_value_t> *insn_retire_port;
	port_t<insn_squash_value_t> *insn_squash_port;
	port_t<mem_read_value_t> *mem_read_port;
	port_t<mem_write_value_t> *mem_write_port;
	port_t<mem_retire_value_t> *mem_retire_port;
	port_t<mem_ready_value_t> *mem_ready_port;
	port_t<reg_write_value_t> *reg_write_port;
};

#endif