#include "pending_handler.h"

void pending_handler_t::check_pending() {
	for(auto it : pending_events) it->check();
}
