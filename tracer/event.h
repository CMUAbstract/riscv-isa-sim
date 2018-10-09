#ifndef EVENT_H
#define EVENT_H

#include <vector>
#include <algorithm>

#include <stat/stat.h>

typedef uint64_t cycle_t;

struct event_base_t {
	virtual void handle() = 0;
	virtual ~event_base_t() {}
	cycle_t cycle = 0;
	cycle_t latency = 0;
};

#define HANDLER void handle(){ this->handler->process(this); }

template <typename T>
struct event_t: public event_base_t {
	event_t() {}
	event_t(T *_handler) : handler(_handler) {}
	~event_t() {}
	T* handler = nullptr;
	virtual void handle() = 0;
};

struct event_comparator_t {
	bool operator()(const event_base_t *a,const event_base_t* b) const{
		return a->cycle <= b->cycle;
	}
};

struct event_list_t {
	void push_back(event_base_t *e) {
		events.push_back(e);
		std::push_heap(events.begin(), events.end(), event_comparator_t()); 
	}
	event_base_t *pop_back() {
		auto tmp = events.front();
		std::pop_heap(events.begin(), events.end(), event_comparator_t());
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
	std::vector<event_base_t *> events;
	bool ready_flag = false;
};

class component_t {
public:
	component_t(io::json _config, event_list_t *_events) 
		: config(_config), events(_events), clock("clock", "") {
		clock.reset();		
	}
	virtual ~component_t(){}
	virtual void process(event_t<component_t> *event) {}
	void add_child(component_t *child) { children.push_back(child); }
	void add_parent(component_t *parent) { parents.push_back(parent); }
protected:
	io::json config;
	event_list_t *events;
	std::vector<component_t *> children;
	std::vector<component_t *> parents;
	counter_stat_t<cycle_t> clock;
};

#endif