#ifndef EVENT_H
#define EVENT_H

#include <string>
#include <iostream>

#include <fesvr/memif.h>
#include <common/decode.h>

struct value_base_t : public io::serializable {
	value_base_t(cycle_t _cycle) : cycle(_cycle) {}
	virtual ~value_base_t() {}

	virtual std::string get_name() const = 0;
	virtual io::json to_json() const = 0;
	virtual std::string to_string() const = 0;
	
	cycle_t cycle = 0;
};

template <typename T>
struct value_t: public value_base_t {
	value_t(T _data)
		: value_base_t(0), data(_data) {}
	value_t(T *_data, cycle_t _cycle)
		: value_base_t(_cycle), data(_data) {}
	virtual ~value_t() {}
	T data;
};

#endif
