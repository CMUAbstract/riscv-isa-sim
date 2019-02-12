#ifndef EXCEPT_H
#define EXCEPT_H

#include <exception>
#include <string>

#include <common/decode.h>

struct soft_except_t: public std::exception {
	const char *what() {
		return "Soft failure";
	}
	reg_t minstret;
};

struct hard_except_t: public std::exception {
	const char *what() {
		return "Hard failure";
	}
	reg_t minstret;
};

#endif