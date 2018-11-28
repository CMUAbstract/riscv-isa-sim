#ifndef COMPONENT_H
#define COMPONENT_H

#include <string>
#include <sstream>

#include <common/decode.h>
#include <stat/stat.h>
#include <io/io.h>
#include <hstd/vector.h>
#include <hstd/map.h>

#define TIME_VIOLATION_CHECK 													\
	assert_msg(event->cycle >= clock.get(), 									\
		"Timing violation e%lu < c%lu", event->cycle, clock.get());				\
	clock.set(event->cycle);

class event_hmap_t;
class component_base_t: public io::serializable {
public:
	component_base_t(std::string _name, io::json _config, event_hmap_t *_events) 
		: name(_name), config(_config), events(_events), clock("clock", "") {
		clock.reset();		
	}
	virtual ~component_base_t() {}
	virtual void init() {}
	virtual void reset() { clock.reset(); }
	virtual io::json to_json() const;

	virtual void enqueue(hstd::vector *vec) = 0;
	virtual void insert(std::string key, hstd::map<std::string> *map) = 0;
	virtual void add_child(std::string name, component_base_t *child) = 0;
	virtual void add_parent(std::string name, component_base_t *parent) = 0;

	std::string get_name() { return name; }
	io::json get_config() { return config; }
	cycle_t get_clock() { return clock.get(); }
protected:
	std::string name;
	io::json config;
	event_hmap_t *events;
	hstd::map<std::string> children;
	hstd::map<std::string> parents;
	counter_stat_t<cycle_t> clock;
};

template <class CHILD, class...EVENT_HANDLERS>
class component_t: public component_base_t, public EVENT_HANDLERS... {
public:
	using component_base_t::component_base_t;
	virtual void enqueue(hstd::vector *vec) {
		vec->push_back<CHILD *>(static_cast<CHILD *>(this));
		(..., vec->push_back<EVENT_HANDLERS *>(this));
	}
	virtual void insert(std::string key, hstd::map<std::string> *map) {
		map->insert<CHILD *>(key, static_cast<CHILD *>(this));
		(..., map->insert<EVENT_HANDLERS *>(key, this));
	}
	virtual void add_child(std::string name, component_base_t *child) {
		child->insert(name, &children);
	}
	virtual void add_parent(std::string name, component_base_t *parent) {
		parent->insert(name, &parents);
	}
};

#endif