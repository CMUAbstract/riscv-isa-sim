#include "value.h"

std::ostream& operator<<(std::ostream& os, const value_base_t& obj) {
	os << obj.to_string();
	return os;
}