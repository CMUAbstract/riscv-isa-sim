#include "pending_handler.h"

void pending_handler_t::check_pending() {
	auto it = pending_events.begin(ref_events);
	while(it != pending_events.end(ref_events)) {
		it->check();
		it++;
	}
}
