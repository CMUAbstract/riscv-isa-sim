#include "event.h"

#include <algorithm>

void event_heap_t::heapify() {
	std::make_heap(events_heap.begin(), events_heap.end(), 
		event_heap_t::comparator_t());	
}

void event_heap_t::push_back(event_base_t *e) {
	events_heap.push_back(e);
	events_set.insert(e);
	e->priority = arrival++;
	std::push_heap(events_heap.begin(), events_heap.end(), 
		event_heap_t::comparator_t()); 
#if 0
	for(auto event : events_heap)
		std::cerr << event->get_name() << " " << event->cycle << std::endl; 
	std::cerr << std::endl;
#endif
}

event_base_t *event_heap_t::pop_back() {
	auto tmp = events_heap.front();
	std::pop_heap(events_heap.begin(), events_heap.end(), 
		event_heap_t::comparator_t());
	events_heap.pop_back();
	return tmp;
}