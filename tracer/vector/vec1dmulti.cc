#include "vec1dmulti.h"

#include "mem_event.h"
#include "vector_event.h"
#include "pending_event.h"

vec1dmulti_t::vec1dmulti_t(std::string _name, io::json _config, 
	event_heap_t *_events) : vcu_t(_name, _config, _events) {
	JSON_CHECK(int, config["window_size"], window_size);
	assert_msg(window_size > 0, "Window size must be greater than zero");
	JSON_CHECK(bool, config["src_forwarding"], src_forwarding);
	JSON_CHECK(int, config["rf_ports"], rf_ports);
	assert_msg(window_size > 0, "Window size must be greater than zero");
}

io::json vec1dmulti_t::to_json() const {
	return vcu_t::to_json();
}

void vec1dmulti_t::reset(reset_level_t level) {
	vcu_t::reset(level);
}

void vec1dmulti_t::process(vector_exec_event_t *event) {

}

void vec1dmulti_t::process(pe_exec_event_t *event) {
	
}

void vec1dmulti_t::process(pe_ready_event_t *event) {
	
}