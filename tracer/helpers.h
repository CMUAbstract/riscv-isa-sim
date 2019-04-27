#ifndef HELPERS_H
#define HELPERS_H

#include <string>
#include <sstream>

static inline std::string hexify(const uint64_t v) {
	std::ostringstream os;
	os << "0x" << std::hex << v << std::endl;
	return os.str();
}

#endif