#ifndef BRANCH_PREDICTOR_H
#define BRANCH_PREDICTOR_H

#include <fesvr/memif.h>

#include "module.h"
#include "port.h"

class predict_event_t;
class check_predict_event_t;
class branch_event_t;
class insn_squash_event_t;
class branch_predictor_t : public module_t {
public:
	branch_predictor_t(std::string _name, io::json _config, scheduler_t *_scheduler);
	virtual void init() {}
	virtual void reset() {}
	virtual bool predict(addr_t cur_pc) = 0;
	virtual void update(addr_t cur_pc, addr_t next_pc) = 0;
protected:
	port_t<predict_event_t *> *predict_port;
	port_t<check_predict_event_t *> *check_predict_port;
	port_t<insn_squash_event_t *> *insn_squash_port;
	port_t<branch_event_t *> *branch_port;
protected:
	bool check_predict(addr_t cur_pc, addr_t next_pc);
};

class local_predictor_t : public branch_predictor_t {
public:
	using branch_predictor_t::branch_predictor_t;
	virtual void init();
	void reset();
	bool predict(addr_t cur_pc);
	void update(addr_t cur_pc, addr_t next_pc);
protected:
	std::vector<addr_t> predictors;
	std::vector<uint8_t> counters;
	uint16_t slots = 64;
	uint16_t mask;
};

class global_predictor_t : public branch_predictor_t {
public:
	using branch_predictor_t::branch_predictor_t;
	void reset();
	bool predict(addr_t cur_pc);
	void update(addr_t cur_pc, addr_t next_pc);
protected:
	uint8_t predictor = 2;
};

class tournament_predictor_t : public branch_predictor_t {
public:
	tournament_predictor_t(std::string _name, io::json _config, scheduler_t *_scheduler)
		: branch_predictor_t(_name, _config, _scheduler), 
		localp(_name, _config, _scheduler), 
		globalp(_name, _config, _scheduler) {}
	void reset();
	bool predict(addr_t cur_pc);
	void update(addr_t cur_pc, addr_t next_pc);
protected:
	local_predictor_t localp;
	global_predictor_t globalp;
	uint8_t selector = 2;
	bool local = true;
	bool global = true;
};

#endif