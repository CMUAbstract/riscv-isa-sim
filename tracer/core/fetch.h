#ifndef FETCH_H
#define FETCH_H

#include <hstd/memory.h>
#include <vector>

#include "port.h"
#include "module.h"
#include "insn_info.h"

class insn_fetch_event_t;
class insn_decode_event_t;
class insn_squash_event_t;
class mem_read_event_t;
class mem_retire_event_t;
class fetch_t : public module_t {
public:
	fetch_t(std::string _name, io::json _config, scheduler_t *_scheduler);
private:
	bool occupied = false;
	std::vector<hstd::shared_ptr<insn_info_t>> insns;
	size_t idx = 0;
private:
	persistent_port_t<bool> *bp_port;
	signal_port_t<insn_fetch_event_t *> *insn_fetch_port;
	signal_port_t<insn_decode_event_t *> *insn_decode_port;
	signal_port_t<insn_squash_event_t *> *insn_squash_port;
	signal_port_t<mem_read_event_t *> *mem_read_port;
	signal_port_t<mem_retire_event_t *> *mem_retire_port;
};

#endif