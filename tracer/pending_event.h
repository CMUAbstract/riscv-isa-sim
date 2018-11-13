#ifndef PENDING_EVENT
#define PENDING_EVENT

#include <sstream>

#include <hcontainer/hvec.h>

#include "event.h"

struct pending_event_t: public event_t<pending_handler_t, event_base_t *> {
	using event_t<pending_handler_t, event_base_t *>::event_t;
	std::string to_string() {
		std::ostringstream o;
		o << "pending_event (" << data->to_string() << ")" << std::endl; 
		return o.str();
	}
	HANDLER;
	bool check() {
		return dependencies.size() == 0;
	}
	template<class T>
	bool check(T *event) {
		for(auto it : dependencies.raw<T>()) {
			if(*it(event)) dependencies.erase<T>(it);
		}
		return dependencies.size() == 0;
	}
	template<class T>
	void add_dependence(std::function<bool(T *)> dep) {
		dependencies.push_back(dep);
	}
	hvec dependencies;
};

#endif