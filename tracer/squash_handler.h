#ifndef SQUASH_HANDLER_H
#define SQUASH_HANDLER_H

#include "event.h"

struct squash_event_t;
class squash_handler_t{
public:
	void set_ref(event_heap_t *_ref_events) { ref_events = _ref_events; }
	virtual void process(squash_event_t *event) = 0;
protected:
	void register_squashed(const std::string &key, event_base_t *event) {
		squashed_events[key].insert(event);
	}
	void squash(const std::string &key) {
		auto it = squashed_events[key].begin(ref_events);
		while(it != squashed_events[key].end(ref_events)) {
			it->squashed = true;
			it++;
		}
	}
	void clear_squash(const std::string &key) { squashed_events[key].clear(); }
protected:
	std::map<std::string, eventref_set_t<event_base_t *>> squashed_events;
private:
	event_heap_t *ref_events = nullptr;
};

#endif