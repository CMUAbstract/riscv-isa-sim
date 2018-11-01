#ifndef EVENT_H
#define EVENT_H

#include <string>
#include <map>
#include <vector>
#include <set>
#include <algorithm>

#include <stat/stat.h>
#include "log.h"
#include "misc.h"

typedef uint64_t cycle_t;

struct event_base_t {
	event_base_t() {}
	event_base_t(cycle_t _cycle): cycle(_cycle), guid(gen_guid()) {}
	event_base_t(cycle_t _cycle, cycle_t _latency) 
		: cycle(_cycle), latency(_latency), guid(gen_guid()) {}
	virtual void handle() = 0;
	virtual std::string to_string() { return "generic_event"; }
	virtual ~event_base_t() {}
	cycle_t cycle = 0;
	cycle_t latency = 0;
	size_t guid = 0;
	bool ready_gc = true;
};

#if 1
#define HANDLER 																\
	void handle() { 															\
		std::cout << this->handler->get_name();									\
		std::cout << " => ";													\
		std::cout << this->to_string();											\
		std::cout << std::endl;													\
		this->handler->process(this);											\
	}
#else
#define HANDLER void handle() { this->handler->process(this); }
#endif

template <typename T, typename K>
struct event_t: public event_base_t {
	event_t() {}
	event_t(T *_handler): handler(_handler) {}
	event_t(T *_handler, event_base_t *event): handler(_handler) {
		deps.insert(event->guid);
	}
	event_t(T *_handler, K _data): handler(_handler), data(_data) {}
	event_t(T *_handler, K _data, event_base_t *event)
		: handler(_handler), data(_data) {
		deps.insert(event->guid);
	}
	event_t(T *_handler, K _data, cycle_t _cycle)
		: event_base_t(_cycle), handler(_handler), data(_data) {}
	event_t(T *_handler, K _data, cycle_t _cycle, event_base_t *event)
		: event_base_t(_cycle), handler(_handler), data(_data) {
		deps.insert(event->guid);
	}
	event_t(T *_handler, K _data, cycle_t _cycle, cycle_t _latency)
		: event_base_t(_cycle, _latency), handler(_handler), data(_data) {}
	event_t(T *_handler, K _data, cycle_t _cycle, cycle_t _latency, event_base_t *event)
		: event_base_t(_cycle, _latency), handler(_handler), data(_data) {
		deps.insert(event->guid);	
	}
	~event_t() {}
	T* handler = nullptr;
	K data;
	std::set<size_t> deps;
};

struct event_comparator_t {
	bool operator()(const event_base_t *a,const event_base_t* b) const{
		return a->cycle >= b->cycle;
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
	void mark_event(event_base_t *event) { marked_events.push_back(event); }
	size_t gc_size(void) { return marked_events.size(); }
	void gc(void) { 
		for(auto it : marked_events) delete it; 
		marked_events.clear();
	}
private:
	std::vector<event_base_t *> events;
	std::vector<event_base_t *> marked_events;
	bool ready_flag = false;
};

#define TIME_VIOLATION_CHECK 													\
	assert_msg(event->cycle >= clock.get(), "Timing violation");				\
	clock.set(event->cycle);

#define GC_EVENT(e) MARK_EVENT(e)
#define MARK_EVENT(e) events->mark_event(e)

template <typename T>
class signal_event_t;
class component_t {
public:
	component_t(std::string _name, io::json _config, event_list_t *_events) 
		: name(_name), config(_config), events(_events), clock("clock", "") {
		clock.reset();		
	}
	virtual void init() {}
	virtual ~component_t() {}
	template <typename T>
	void process(signal_event_t<T> *event) {
		assert_msg(1 == 0, "Generic signal event being handled");
	}
	void add_child(std::string name, component_t *child) {
		children.insert({name, child});
	}
	void add_parent(std::string name, component_t *parent) {
		parents.insert({name, parent});
	}
	std::string get_name() { return name; }
	io::json get_config() { return config; }
	event_list_t *get_events() { return events; }
protected:
	std::string name;
	io::json config;
	event_list_t *events;
	event_list_t *gc_events;
	std::map<std::string, component_t *> children;
	std::map<std::string, component_t *> parents;
	counter_stat_t<cycle_t> clock;
};

template <typename T>
struct signal_event_t: public event_t<component_t, T> {
	using event_t<component_t, T>::event_t;
};

#endif