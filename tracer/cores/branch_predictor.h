#ifndef BRANCH_PREDICTOR_H
#define BRANCH_PREDICTOR_H

#include <fesvr/memif.h>
#include <common/decode.h>
#include <io/io.h>

class branch_predictor_t {
public:
	branch_predictor_t() {}
	branch_predictor_t(io::json config) {}
	bool check_branch(insn_bits_t opc);
	virtual void reset() = 0;
	virtual bool predict(addr_t cur_pc) = 0;
	virtual bool check_predict(addr_t cur_pc, addr_t next_pc);
	virtual void update(addr_t cur_pc, addr_t next_pc) = 0;
};

class local_predictor_t: public branch_predictor_t {
public:
	local_predictor_t(io::json config);
	void reset();
	bool predict(addr_t cur_pc);
	void update(addr_t cur_pc, addr_t next_pc);
protected:
	std::vector<addr_t> predictors;
	std::vector<uint8_t> counters;
	uint16_t slots = 64;
	uint16_t mask;
};

class global_predictor_t: public branch_predictor_t {
public:
	using branch_predictor_t::branch_predictor_t;
	void reset();
	bool predict(addr_t cur_pc);
	void update(addr_t cur_pc, addr_t next_pc);
protected:
	uint8_t predictor = 2;
};

class tournament_predictor_t: public branch_predictor_t {
public:
	tournament_predictor_t(io::json config)
		: localp(config), globalp(config) {}
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

template <typename T>
branch_predictor_t *create_branch_predictor(io::json _config) {
	return new T(_config);
}

const std::map<std::string, branch_predictor_t*(*)(io::json _config)> 
branch_predictor_type_map = {
	{"local", &create_branch_predictor<local_predictor_t>},
	{"global", &create_branch_predictor<global_predictor_t>},
	{"tournament", &create_branch_predictor<tournament_predictor_t>}
};

#endif