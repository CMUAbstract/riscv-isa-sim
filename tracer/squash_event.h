#ifndef SQUASH_EVENT_H
#define SQUASH_EVENT_H

#include <sstream>
#include <vector>

#include "event.h"
#include "squash_handler.h"

struct squash_event_t: public event_t<squash_handler_t, std::vector<std::string>> {
	using event_t<squash_handler_t, std::vector<std::string>>::event_t;
	std::string to_string() {
		std::ostringstream o;
		o << "squash_event (" << cycle << ", ";
		for(auto it = data.begin(); it < data.end(); ++it) {
			if(it != data.end() - 1) o << *it << ", ";
			else o << *it;
		}
		o << ")";
		return o.str();
	}
	std::string get_name() { return "squash_event"; }
	HANDLER;
};

#endif