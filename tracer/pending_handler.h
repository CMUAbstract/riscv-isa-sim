#ifndef PENDING_HANDLER
#define PENDING_HANDLER

#include <set>

class pending_event_t;
class pending_handler_t {
public:
	virtual void process(pending_event_t *event) = 0;
protected:
	void register_pending(pending_event_t *event) {
		pending_events.insert(event);
	}
	void remove_pending(pending_event_t *event) {
		auto it = pending_events.find(event);
		if(it != pending_events.end()) pending_events.erase(it);
	}
	template<class T>
	void check_pending(T event);
	void check_pending();
private:
	std::set<pending_event_t *> pending_events;
};

#include "pending_event.h"
template<class T>
void pending_handler_t::check_pending(T event) {
	for(auto it : pending_events) it->check(event);
}

#endif