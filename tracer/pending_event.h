#ifndef PENDING_EVENT
#define PENDING_EVENT

#include <sstream>
#include <vector>

#include <hcontainer/hvec.h>

#include "event.h"
#include "pending_handler.h"
#if 1
#include "component.h"
#endif

struct pending_event_t: public event_t<pending_handler_t, event_base_t *> {
	pending_event_t(pending_handler_t *_handler, event_base_t *_data, cycle_t _cycle)
		: event_t(_handler, _data, _cycle) { pending = true; }
	std::string to_string() {
		std::ostringstream o;
		o << "pending_event (" << cycle << ", " << dependencies.size();
		if(data == nullptr) { 
			o << ")";
			return o.str();
		}
		o << ", " << data->to_string() << ")"; 
		return o.str();
	}
	std::string get_name() { return "pending_event"; }
	HANDLER;
	bool resolved() { return dependencies.size() == 0; }
	// Endless loop
	template<class T>
	bool check(T event) {
		for(auto it = dependencies.begin<std::function<bool(T)>>();
			it != dependencies.end<std::function<bool(T)>>(); ++it) {
			if((*it)(event)) {
				it = dependencies.erase<std::function<bool(T)>>(it);
				--it;
			}
		}
		return dependencies.size() == 0;
	}
	bool check() {
		for(auto it = dependencies.begin<std::function<bool()>>();
			it != dependencies.end<std::function<bool()>>(); ++it) {
			if((*it)()) {
				it = dependencies.erase<std::function<bool()>>(it);
				--it;
			}
		}
		return dependencies.size() == 0;
	}
	template<class T>
	void add_dependence(std::function<bool(T)> dep) {
		dependencies.push_back(dep);
	}
	void add_dependence(std::function<bool()> dep) {
		dependencies.push_back(dep);
	}
	void finish() { for(auto it : finis) it(); }
	void add_fini(std::function<void()> fini) { finis.push_back(fini); }
	hvec dependencies;
	std::vector<std::function<void()>> finis;
};

#endif