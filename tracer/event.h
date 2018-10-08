#ifndef EVENT_H
#define EVENT_H

#include <vector>
#include <algorithm>

#include <stat/stat.h>

typedef uint64_t cycle_t;

class component_t;
struct event_t {
	event_t() {}
	bool operator<(const event_t &e) const {
       return cycle < e.cycle;
    }
	cycle_t cycle = 0;
	cycle_t latency = 0;
	component_t *component = nullptr;
};

struct event_list_t {
	void push_back(event_t e) {
		events.push_back(e);
		std::push_heap(events.begin(), events.end()); 
	}
	event_t pop_back() {
		auto tmp = events.front();
		std::pop_heap(events.begin(), events.end());
		events.pop_back();
		return tmp;
	}
	bool empty() {
		return events.size() == 0;
	}
	bool ready() { return ready_flag; }
	void set_ready() { ready_flag = true; }
	void set_ready(bool val) { ready_flag = val; }

private:
	std::vector<event_t> events;
	bool ready_flag = false;
};

class component_t {
public:
	component_t(io::json _config, event_list_t *_events) 
		: config(_config), events(_events) {}
	virtual ~component_t(){}
	virtual void process(event_t event) = 0;
	void add_child(component_t *child) { children.push_back(child); }
	void add_parent(component_t *parent) { parents.push_back(parent); }
protected:
	io::json config;
	event_list_t *events;
	std::vector<component_t *> children;
	std::vector<component_t *> parents;
	cycle_t cycle = 0;
};

#endif