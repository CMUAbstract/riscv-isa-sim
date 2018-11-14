#include "pending_handler.h"

#include "pending_event.h"

template<class T>
void pending_handler_t::check_pending(T event) {
	for(auto it : pending_events) it->check(event);
}

void pending_handler_t::check_pending() {
	for(auto it : pending_events) it->check();
}