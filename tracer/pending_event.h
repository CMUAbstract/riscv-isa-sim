#ifndef PENDING_EVENT
#define PENDING_EVENT

#include <sstream>
#include <vector>

#include <hstd/vector.h>

#include "event.h"
#include "pending_handler.h"
#include "component.h"

struct pending_event_t: public event_t<pending_handler_t, event_base_t *> {
	pending_event_t(pending_handler_t *_handler, event_base_t *_data, cycle_t _cycle)
		: event_t(_handler, _data, _cycle) { pending = true; }
	~pending_event_t() { if(data != nullptr) delete data; }
	std::string to_string() {
		std::ostringstream o;
		o << "pending_event (" << cycle << ", ";
		o << signal_deps << ", " << persistent_deps;
		if(data == nullptr) { 
			o << ")";
			return o.str();
		}
		o << ", " << data->to_string() << ")"; 
		return o.str();
	}
	std::string get_name() { return "pending_event"; }
	HANDLER;
	bool resolved() {
		return (persistent_deps == 0 || check()) && signal_deps == 0; 
	}
	template<class T>
	void check(T event) {
		auto it = deps.begin<std::function<bool(T)>>();	
		while(it != deps.end<std::function<bool(T)>>()) {
			if((*it)(event)) {
				it = deps.erase<std::function<bool(T)>>(it);
				if(signal_deps > 0) signal_deps--;
			} else ++it;
		}
	}
	template<class T>
	void add_dep(std::function<bool(T)> dep) {
		signal_deps++;
		deps.push_back(dep); 
	}
	void add_dep(std::function<bool()> dep) { 
		persistent_deps++;
		deps.push_back(dep); 
	}
	void finish() { for(auto it : finis) it(); }
	void add_fini(std::function<void()> fini) { finis.push_back(fini); }
private:
	bool check() {
		for(auto it : deps.raw<std::function<bool()>>()) 
			if(!it()) return false;
		return true;
	}
private:
	hstd::vector deps;
	uint32_t signal_deps = 0;
	uint32_t persistent_deps = 0;
	std::vector<std::function<void()>> finis;
};

#endif