#include "branch_predictor.h"

bool branch_predictor_t::check_branch(insn_bits_t opc) {
	switch(opc) {
		case MATCH_BNE:
		case MATCH_BEQ:
		case MATCH_C_BNEZ:
		case MATCH_BGEU:
		case MATCH_BLTU:
		case MATCH_C_BEQZ:
		case MATCH_BGE:
		case MATCH_BLT: return true;
	};
	return false;
}

bool branch_predictor_t::check_predict(addr_t cur_pc, addr_t next_pc) {
	bool prediction = predict(cur_pc);
	if(prediction && next_pc == cur_pc + 4) return false;
	if(!prediction && next_pc != cur_pc + 4) return false;
	return true;
}

local_predictor_t::local_predictor_t(io::json config) {
	JSON_CHECK(int, config["slots"], slots);
	mask = slots - 1;
	predictors.resize(slots);
	counters.resize(slots);
}

bool local_predictor_t::predict(addr_t cur_pc) {
	uint32_t idx = cur_pc & mask;
	if(predictors[idx] == cur_pc && counters[idx] > 1) return true;
	return false;
}

void local_predictor_t::update(addr_t cur_pc, addr_t next_pc) {
	uint32_t idx = cur_pc & mask;
	if(predictors[idx] != cur_pc) {
		predictors[idx] = cur_pc;
		if(next_pc == cur_pc + 4) counters[idx] = 1;
		else if(next_pc != cur_pc + 4) counters[idx] = 2;
	} else if(next_pc == cur_pc + 4 && counters[idx] > 0) counters[idx]--;
	else if(next_pc != cur_pc + 4 && counters[idx] < 3) counters[idx]++;
}

bool global_predictor_t::predict(addr_t cur_pc) {
	if(predictor > 1) return true;
	return false;
}

void global_predictor_t::update(addr_t cur_pc, addr_t next_pc) {
	if(next_pc == cur_pc + 4 && predictor > 0) predictor--;
	if(next_pc != cur_pc + 4 && predictor < 3) predictor++;
}

bool tournament_predictor_t::predict(addr_t cur_pc) {
	local = localp.predict(cur_pc);
	global = globalp.predict(cur_pc);
	if(local == global) return local;
	else if(selector < 2) return local;
	else if(selector > 1) return global;
	return false;
}

void tournament_predictor_t::update(addr_t cur_pc, addr_t next_pc) {
	localp.update(cur_pc, next_pc);
	globalp.update(cur_pc, next_pc);
	if(!local && global && selector < 3) selector++;
	if(local && !global && selector > 0) selector--;
}