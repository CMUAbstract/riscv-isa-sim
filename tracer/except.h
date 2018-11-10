#ifndef EXCEPT_H
#define EXCEPT_H

#include <exception>
#include <string>

#include <common/decode.h>

struct intermittent_except_t: public std::exception {
	const char *what() {
		return "Triggering intermittent failure";
	}
	reg_t minstret;
};

#endif