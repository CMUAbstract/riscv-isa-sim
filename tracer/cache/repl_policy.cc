#include "repl_policy.h"

#include <algorithm>

#include "mem_event.h"

void lru_repl_policy_t::update(uint32_t id, mem_event_t *req) {
	timestamps[id] = timestamp++;
}

uint32_t lru_repl_policy_t::rank(mem_event_t *req, std::vector<repl_cand_t> *cands) {
	uint32_t best_score = timestamps[**cands->begin()];
	uint32_t best_cand = **cands->begin();
	for(auto it : *cands) {
		auto score = timestamps[*it];
		if(score < best_score) {
			best_score = score;
			best_cand = *it; 
		}
	}
	return best_cand;
}

void lru_repl_policy_t::replaced(uint32_t id) {
	timestamps[id] = 0;
}