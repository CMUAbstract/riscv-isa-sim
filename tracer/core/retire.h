#ifndef RETIRE_H
#define RETIRE_H

#include <stat/stat.h>

#include "port.h"
#include "module.h"

class insn_retire_value_t;
class retire_t : public module_t {
public:
	retire_t(std::string _name, io::json _config, scheduler_t *_scheduler);
	io::json to_json() const;
private:
	port_t<insn_retire_value_t> *insn_retire_port;
	counter_stat_t<uint64_t> retired_insns;
};

#endif