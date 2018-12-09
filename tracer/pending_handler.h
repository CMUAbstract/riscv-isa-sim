#ifndef PENDING_HANDLER
#define PENDING_HANDLER

#include <map>

#include "event.h"

struct pending_event_t;
class pending_handler_t {
public:
	void set_ref(event_heap_t *_ref_events) { ref_events = _ref_events; }
	virtual void process(pending_event_t *event);
protected:
	void register_pending(pending_event_t *event) {
		pending_events.insert({event, event});
	}
	pending_event_t *promote_pending(event_base_t *event, std::function<bool()> cond);
	template<class T>
	void check_pending(T event);
	void clear_pending() { pending_events.clear(); }
protected:
	std::map<eventref_t, pending_event_t *> pending_events;
private:
	event_heap_t *ref_events = nullptr;
};

#include "pending_event.h"
template<class T>
void pending_handler_t::check_pending(T event) {
	std::vector<std::pair<eventref_t, pending_event_t *>> alive_items;
	for(auto it : pending_events) {
		if(ref_events->count(it.first) != 0) {
			alive_items.push_back(it);
			it.second->check(event);
		}
	}
	pending_events.clear();
	for(auto it : alive_items) pending_events.insert(it);
}

#endif