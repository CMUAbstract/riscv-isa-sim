#ifndef EVENT_H
#define EVENT_H

#include <string>
#include <iostream>

#include <fesvr/memif.h>
#include <common/decode.h>

struct event_base_t : public io::serializable {
	event_base_t(cycle_t _cycle) : cycle(_cycle) {}
	virtual ~event_base_t() {}

	virtual std::string get_name() const = 0;
	virtual io::json to_json() const = 0;
	virtual std::string to_string() const = 0;
	
	cycle_t cycle = 0;
};

template <typename T>
struct event_t: public event_base_t {
	event_t(T _data)
		: event_base_t(0), data(_data) {}
	event_t(T *_data, cycle_t _cycle)
		: event_base_t(_cycle), data(_data) {}
	virtual ~event_t() {}
	T data;
};

struct event_comparator_t {
	bool operator()(const event_base_t *a,const event_base_t *b) const{
		return a->cycle > b->cycle;
	}	
};

#endif
