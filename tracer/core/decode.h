#ifndef DECODE_H
#define DECODE_H

#include "port.h"
#include "module.h"

class insn_decode_event_t;
class insn_exec_event_t;
class insn_squash_event_t;
class reg_read_event_t;
class decode_t : public module_t {
public:
	decode_t(std::string _name, io::json _config, scheduler_t *_scheduler);
private:
	bool occupied = false;
private:
	persistent_port_t<bool> *bp_input_port;
	persistent_port_t<bool> *bp_output_port;
	signal_port_t<insn_decode_event_t *> *insn_decode_port;
	signal_port_t<insn_exec_event_t *> *insn_exec_port;
	signal_port_t<insn_squash_event_t *> *insn_squash_input_port;
	signal_port_t<insn_squash_event_t *> *insn_squash_output_port;
	signal_port_t<reg_read_event_t *> *reg_read_port;
};

#endif