#include "value.h"

#include <algorithm>

void value_heap_t::heapify() {
	std::make_heap(values_heap.begin(), values_heap.end(), 
		value_heap_t::comparator_t());	
}

void value_heap_t::push_back(value_base_t *e) {
	values_heap.push_back(e);
	values_set.insert(e);
	e->priority = arrival++;
	std::push_heap(values_heap.begin(), values_heap.end(), 
		value_heap_t::comparator_t()); 
#if 0
	for(auto value : values_heap)
		std::cerr << value->get_name() << " " << value->cycle << std::endl; 
	std::cerr << std::endl;
#endif
}

value_base_t *value_heap_t::pop_back() {
	auto tmp = values_heap.front();
	std::pop_heap(values_heap.begin(), values_heap.end(), 
		value_heap_t::comparator_t());
	values_heap.pop_back();
	return tmp;
}