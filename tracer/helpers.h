#ifndef HELPERS_H
#define HELPERS_H

#include <string>
#include <sstream>

static inline std::string hexify(const std::string& str) {
	std::ostringstream os;
	os << "0x" << std::hex << str << std::endl;
	return os.str();
}

#endif