#ifndef PENDING_HANDLER
#define PENDING_HANDLER

#include <set>
#include "event.h"

class pending_event_t;
class pending_handler_t {
public:
	virtual void process(pending_event_t *event) = 0;
protected:
	void register_pending(pending_event_t *event) {
		pending_events.insert(event);
	}
	void remove_pending(eventref_t event);
	template<class T>
	void check_pending(T event);
	void check_pending();
	void clear_pending() { pending_events.clear(); }
protected:
	eventref_set_t<pending_event_t *> pending_events;
};

#include "pending_event.h"
template<class T>
void pending_handler_t::check_pending(T event) {
	for(auto it : pending_events) it->check(event);
}

#endif