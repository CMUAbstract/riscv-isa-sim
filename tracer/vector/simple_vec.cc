#include "simple_vec.h"

#include "vector_event.h"
#include "pending_event.h"

simple_vec_t::simple_vec_t(std::string _name, io::json _config, event_heap_t *_events)
	: vcu_t(_name, _config, _events) {}

void simple_vec_t::process(vector_exec_event_t *event) {

}

void simple_vec_t::process(pe_exec_event_t *event) {

}

void simple_vec_t::process(pe_ready_event_t *event) {

}

void simple_vec_t::process(pending_event_t *event) {

}