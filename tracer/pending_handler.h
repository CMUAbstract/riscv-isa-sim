#ifndef PENDING_HANDLER
#define PENDING_HANDLER

#include "event.h"

class pending_event_t;
class pending_handler_t {
public:
	void set_ref(event_heap_t *_ref_events) { ref_events = _ref_events; }
	virtual void process(pending_event_t *event) = 0;
protected:
	void register_pending(pending_event_t *event) {
		pending_events.insert(event);
	}
	template<class T>
	void check_pending(T event);
	void check_pending();
	void clear_pending() { pending_events.clear(); }
protected:
	eventref_set_t<pending_event_t *> pending_events;
private:
	event_heap_t *ref_events = nullptr;
};

#include "pending_event.h"
template<class T>
void pending_handler_t::check_pending(T event) {
	auto it = pending_events.begin(ref_events);
	while(it != pending_events.end(ref_events)) {
		it->check(event);
		it++;
	}
}

#endif