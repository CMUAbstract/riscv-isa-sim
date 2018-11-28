#include "event.h"

#include <algorithm>

void event_hmap_t::heapify() {
	std::make_heap(events.begin(), events.end(), event_hmap_t::comparator_t());	
}

void event_hmap_t::push_back(event_base_t * e) {
	events.insert({e, e});
	std::push_heap(events.begin(), events.end(), event_hmap_t::comparator_t()); 
}

event_base_t *event_hmap_t::pop_back() {
	auto tmp = *begin();
	std::pop_heap(events.begin(), events.end(), event_hmap_t::comparator_t());
	erase(end());
	return tmp;
}