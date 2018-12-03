#include "pending_handler.h"

void pending_handler_t::check_pending() {
	std::vector<std::pair<eventref_t, pending_event_t *>> alive_items;
	for(auto it : pending_events) {
		if(ref_events->count(it.first) != 0) {
			alive_items.push_back(it);
			it.second->check();
		}
	}
	pending_events.clear();
	for(auto it : alive_items) pending_events.insert(it);
}
