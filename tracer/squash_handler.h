#ifndef SQUASH_HANDLER_H
#define SQUASH_HANDLER_H

#include <map>
#include "event.h"

struct squash_event_t;
class squash_handler_t{
public:
	void set_ref(event_heap_t *_ref_events) { ref_events = _ref_events; }
	virtual void process(squash_event_t *event) = 0;
protected:
	void register_squashed(const std::string &key, event_base_t *event) {
		squashed_events[key].insert({event, event});
	}
	void squash(const std::string &key) {
		std::vector<std::pair<eventref_t, event_base_t *>> alive_keys;
		for(auto it : squashed_events[key]) {
			if(ref_events->count(it.first) != 0) {
				alive_keys.push_back(it);
				it.second->squashed = true;
			}
		}
		squashed_events[key].clear();
		for(auto it : alive_keys) squashed_events[key].insert(it);
	}
	void clear_squash(const std::string &key) { squashed_events[key].clear(); }
protected:
	std::map<std::string, std::map<eventref_t, event_base_t *>> squashed_events;
private:
	event_heap_t *ref_events = nullptr;
};

#endif