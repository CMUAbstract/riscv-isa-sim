#ifndef COMPONENT_H
#define COMPONENT_H

#include <string>
#include <map>

#include <stat/stat.h>

#define TIME_VIOLATION_CHECK 													\
	assert_msg(event->cycle >= clock.get(), 									\
		"Timing violation e%lu < c%lu", event->cycle, clock.get());				\
	clock.set(event->cycle);

#define GC_EVENT(e) MARK_EVENT(e)
#define MARK_EVENT(e) events->mark_event(e)

typedef uint64_t cycle_t;

class event_list_t;
class component_t {
public:
	component_t(std::string _name, io::json _config, event_list_t *_events) 
		: name(_name), config(_config), events(_events), clock("clock", "") {
		clock.reset();		
	}
	virtual void init() {}
	virtual void reset() {}
	virtual ~component_t() {}
	void add_child(std::string name, component_t *child) {
		children.insert({name, child});
	}
	void add_parent(std::string name, component_t *parent) {
		parents.insert({name, parent});
	}
	std::string get_name() { return name; }
	io::json get_config() { return config; }
	cycle_t get_clock() { return clock.get(); }
protected:
	std::string name;
	io::json config;
	event_list_t *events;
	event_list_t *gc_events;
	std::map<std::string, component_t *> children;
	std::map<std::string, component_t *> parents;
	counter_stat_t<cycle_t> clock;
};

#endif