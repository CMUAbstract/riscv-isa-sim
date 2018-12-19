#include "component.h"

#include "event.h"

io::json component_base_t::to_json() const {
	return io::json::merge_objects(clock, event_counts.to_json());
}

void component_base_t::account(event_base_t *event) {
	if(event_counts.find(event->get_name()) == event_counts.end()) {
		event_counts.insert(event->get_name(), counter_stat_t<uint32_t>());
		event_counts[event->get_name()].reset();
		event_counts[event->get_name()].inc();
		return;
	}
	event_counts[event->get_name()].inc();
}