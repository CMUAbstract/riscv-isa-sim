#ifndef COMPONENT_H
#define COMPONENT_H

#include <array>
#include <string>
#include <sstream>

#include <common/decode.h>
#include <stat/stat.h>
#include <io/io.h>
#include <hstd/vector.h>
#include <hstd/map.h>


#define TIME_VIOLATION_CHECK 													\
	{																			\
		assert_msg(event->cycle >= clock.get(), 								\
			"Timing violation e%lu < c%lu", event->cycle, clock.get());			\
		clock.set(event->cycle);												\
		account(event);															\
	}

#define PARSE_POWER(handle, key)												\
	{																			\
		assert_msg(handle[key].is_array(), "No " key " power provided");		\
		auto vals = handle[key].array_items();									\
		assert_msg(vals.size() == 3, "Dynamic, static, leakage not supplied");	\
		model[key].dynamic = vals[0].double_value();							\
		model[key].steady = vals[1].double_value();								\
		model[key].leakage = vals[2].double_value();							\
	}

struct event_base_t;
class event_heap_t;
class component_base_t: public io::serializable {
public:
	enum power_state_t {ON, BROWN};
public:
	component_base_t(std::string _name, io::json _config, event_heap_t *_events) 
		: name(_name), config(_config), events(_events), clock("clock", ""),
		power("power"), energy("energy"), count("count") {
		clock.reset();		
	}
	virtual ~component_base_t() {}
	virtual void init() {}
	virtual void reset(reset_level_t level=HARD);
	virtual io::json to_json() const;

	virtual void enqueue(hstd::vector *vec) = 0;
	virtual void insert(std::string key, hstd::map<std::string> *map) = 0;
	virtual void add_child(std::string name, component_base_t *child) = 0;
	virtual void add_parent(std::string name, component_base_t *parent) = 0;

	std::string get_name() { return name; }
	io::json get_config() { return config; }
	cycle_t get_clock() { return clock.get(); }
	double get_power(power_state_t state=ON);
	double get_energy();

	void account(event_base_t *event);
protected:
	std::string name;
	io::json config;
	event_heap_t *events;
	hstd::map<std::string> children;
	hstd::map<std::string> parents;
	counter_stat_t<cycle_t> clock;
protected:
	void track_power(std::string key);
	void track_energy(std::string key);
	class duple_stat_t: public stat_t {
	public:
		duple_stat_t() {}
		duple_stat_t(std::string _key1, std::string _key2)
			: duple_stat_t("", "", _key1, _key2) {}
		duple_stat_t(std::string _name, std::string _key1, std::string _key2)
			: duple_stat_t(_name, "", _key1, _key2) {}
		duple_stat_t(std::string _name, std::string _desc, std::string _key1, 
			std::string _key2) : stat_t(_name, _desc), key1(_key1), key2(_key2) {}
		io::json to_json() const;
		void set(double _v1, double _v2);
		std::array<double, 2> get();
	private:
		double v1 = 0.;
		double v2 = 0.;
		std::string key1;
		std::string key2;
	};
	map_stat_t<std::string, duple_stat_t> power;
	map_stat_t<std::string, duple_stat_t> energy;
	map_stat_t<std::string, running_stat_t<counter_stat_t<uint64_t>>> count;
	map_stat_t<std::string, counter_stat_t<uint32_t>> event_counts;
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