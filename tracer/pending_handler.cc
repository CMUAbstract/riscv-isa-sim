#include "pending_handler.h"

#include <stat/stat.h>
#include "pending_event.h"

void pending_handler_t::process(pending_event_t *event) {
	assert_msg(event->cycle >= ref_clock->get(),
		"Timing violation e%lu < c%lu", event->cycle, ref_clock->get());
	ref_clock->set(event->cycle);
	if(!event->resolved()) {
		// Recheck during next cycle
		event->cycle = event->cycle + 1;
		event->ready_gc = false;
		ref_events->push_back(event);
		return;
	}
	event->finish();
	event->ready_gc = true;
	if(event->data != nullptr) {
		event->data->ready_gc = true;
		event->data->cycle = event->cycle;
		ref_events->push_back(event->data);
		event->data = nullptr;
	}
}

pending_event_t *pending_handler_t::promote_pending(
	event_base_t *event, std::function<bool()> cond) {
	if(cond()) {
		event->ready_gc = false;
		auto pending_event = new pending_event_t(this, event, event->cycle + 1);
		pending_event->add_dep([=](){ return !cond(); });
		register_pending(pending_event);
		ref_events->push_back(pending_event);
		return pending_event;
	}
	return nullptr;
}