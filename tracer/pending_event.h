#ifndef PENDING_EVENT
#define PENDING_EVENT

#include <sstream>

#include <hcontainer/hvec.h>

#include "event.h"
#if 1
#include "component.h"
#endif

struct pending_event_t: public event_t<pending_handler_t, event_base_t *> {
	pending_event_t(pending_handler_t *_handler, event_base_t *_data, cycle_t _cycle)
		: event_t(_handler, _data, _cycle) { pending = true; }
	std::string to_string() {
		std::ostringstream o;
		o << "pending_event (" << data->to_string() << ")" << std::endl; 
		return o.str();
	}
	HANDLER;
	bool resolved() { return dependencies.size() == 0; }
	template<class T>
	bool check(T event) {
		for(auto it = dependencies.begin<std::function<bool(T)>>();
			it != dependencies.end<std::function<bool(T)>>(); ++it) {
			if((*it)(event)) dependencies.erase<std::function<bool(T)>>(it);
		}
		return dependencies.size() == 0;
	}
	bool check() {
		for(auto it = dependencies.begin<std::function<bool()>>();
			it != dependencies.end<std::function<bool()>>(); ++it) {
			if((*it)()) dependencies.erase<std::function<bool()>>(it);
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
	hvec dependencies;
};

#endif