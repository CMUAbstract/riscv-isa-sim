#include "pending_handler.h"

void pending_handler_t::remove_pending(eventref_t event) {
	auto it = pending_events.find(event);
	if(it != pending_events.end()) pending_events.erase(it);
}

void pending_handler_t::check_pending() {
	for(auto it : pending_events) it->check();
}
