#ifndef VALUE_H
#define VALUE_H

#include <string>

#include <io/io.h>
#include <hstd/memory.h>

struct value_base_t : public io::serializable {
	virtual ~value_base_t() {}

	virtual std::string get_name() const = 0;
	virtual io::json to_json() const = 0;
	virtual std::string to_string() const = 0;
};

std::ostream& operator<<(std::ostream& os, const value_base_t& obj);

template <typename T>
struct value_t: public value_base_t, hstd::shared_ptr<T> {
	using hstd::shared_ptr<T>::shared_ptr;
};

#endif