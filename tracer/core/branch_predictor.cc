#include "branch_predictor.h"

#include "event/core_event.h"

#if 0
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
#endif

branch_predictor_t::branch_predictor_t(
	std::string _name, io::json _config, scheduler_t *_scheduler)
	: module_t(_name, _config, _scheduler) {

	predict_port = create_port<predict_event_t *>("predict_port");	
	check_predict_port = create_port<check_predict_event_t *>("check_predict_port");	
	insn_squash_port = create_port<insn_squash_event_t *>("insn_squash_port");	
	branch_port = create_port<branch_event_t *>("branch_port");
	init();
}

bool branch_predictor_t::check_predict(addr_t cur_pc, addr_t next_pc) {
	bool prediction = predict(cur_pc);
	if(prediction && next_pc == cur_pc + 4) return false;
	if(!prediction && next_pc != cur_pc + 4) return false;
	return true;
}

void local_predictor_t::init() {
	JSON_CHECK(int, config["slots"], slots);
	mask = slots - 1;
	predictors.resize(slots);
	counters.resize(slots);
}

void local_predictor_t::reset() {
	std::fill(predictors.begin(), predictors.end(), 2);
	std::fill(counters.begin(), counters.end(), 2);
}

bool local_predictor_t::predict(addr_t cur_pc) {
	uint32_t idx = cur_pc & mask;
	if(predictors[idx] == cur_pc && counters[idx] > 1) return true;
	return false;
}

void local_predictor_t::update(addr_t cur_pc, addr_t next_pc) {
	uint32_t idx = cur_pc & mask;
	// std::cout << "0x" << std::hex << cur_pc << " ";
	// std::cout << idx << " " << (uint16_t)counters[idx] << std::endl;
	if(predictors[idx] != cur_pc) {
		predictors[idx] = cur_pc;
		if(next_pc == cur_pc + 4) counters[idx] = 1;
		else if(next_pc != cur_pc + 4) counters[idx] = 2;
	} else if(next_pc == cur_pc + 4 && counters[idx] > 0) {
		counters[idx]--;
	} else if(next_pc != cur_pc + 4 && counters[idx] < 3) {
		counters[idx]++;
	}
}

void global_predictor_t::reset() {
	predictor = 2;	
}

bool global_predictor_t::predict(addr_t cur_pc) {
	if(predictor > 1) return true;
	return false;
}

void global_predictor_t::update(addr_t cur_pc, addr_t next_pc) {
	if(next_pc == cur_pc + 4 && predictor > 0) predictor--;
	if(next_pc != cur_pc + 4 && predictor < 3) predictor++;
}

void tournament_predictor_t::reset() {
	localp.reset();
	globalp.reset();
	selector = 2;
	local = true;
	global = true;
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