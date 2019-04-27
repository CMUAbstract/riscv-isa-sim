#ifndef EXEC_H
#define EXEC_H

#include <hstd/memory.h>

#include "port.h"
#include "module.h"
#include "insn_info.h"

class insn_exec_event_t;
class insn_retire_event_t;
class mem_read_event_t;
class mem_write_event_t;
class mem_retire_event_t;
class mem_ready_event_t;
class reg_write_event_t;
class exec_t : public module_t {
public:
	exec_t(std::string _name, io::json _config, scheduler_t *_scheduler);
private:
	bool occupied = false;
	hstd::shared_ptr<insn_info_t *> cur_insn;
private:
	persistent_port_t<bool> *bp_port;
	signal_port_t<insn_exec_event_t *> *insn_exec_port;
	signal_port_t<insn_retire_event_t *> *insn_retire_port;
	signal_port_t<mem_read_event_t *> *mem_read_port;
	signal_port_t<mem_write_event_t *> *mem_write_port;
	signal_port_t<mem_retire_event_t *> *mem_retire_port;
	signal_port_t<mem_ready_event_t *> *mem_ready_port;
	signal_port_t<reg_write_event_t *> *reg_write_port;
};

#endif