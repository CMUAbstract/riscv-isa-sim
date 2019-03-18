#ifndef SINGLE_ISSUE_3_STAGE_H
#define SINGLE_ISSUE_3_STAGE_H

#include <map>
#include <sstream>

#include <fesvr/memif.h>
#include <stat/stat.h>

#include "event.h"
#include "core.h"

class ram_t;
class vcu_t;
class branch_predictor_t;
class si3stage_core_t: public core_t, public vec_signal_handler_t {
	friend class vcu_t; 
public:
	si3stage_core_t(std::string _name, io::json _config, event_heap_t *_events);
	void init();
	io::json to_json() const;
	void reset(reset_level_t level);
	virtual void enqueue(hstd::vector *vec) {
		component_t::enqueue(vec);
		vec->push_back<vec_signal_handler_t *>(
			static_cast<vec_signal_handler_t *>(this));	
	}
	virtual void insert(std::string key, hstd::map<std::string> *map) {
		component_t::insert(key, map);
		map->insert<vec_signal_handler_t *>(key,
			static_cast<vec_signal_handler_t *>(this));	
	}	
	void buffer_insn(hstd::shared_ptr<timed_insn_t> insn);
	void next_insn();
	void process(insn_fetch_event_t *event);
	void process(insn_decode_event_t *event);
	void process(insn_exec_event_t *event);
	void process(insn_retire_event_t *event);
	void process(squash_event_t *event);
	void process(vec_ready_event_t *event);
	void process(vec_retire_event_t *event);
	void process(reg_read_event_t *event);
private:
	ram_t *icache;
	vcu_t *vcu = nullptr;
	branch_predictor_t *predictor;
	bool last_split = false;
private:
	counter_stat_t<uint32_t> squashes;
};

#endif