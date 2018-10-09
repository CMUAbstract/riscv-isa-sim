#ifndef LOG_H
#define LOG_H

#include <iostream>

#ifndef unlikely
#define unlikely(x) __builtin_expect((x), 0)
#endif

#define assert_msg(cond, args...)												\
	if (unlikely(!(cond))) {													\
	fprintf(stderr, "%sFailed assertion on %d: ",								\
	__FILE__, __LINE__);														\
	fprintf(stderr, args);														\
	fprintf(stderr, "\n");														\
	fflush(stderr);																\
	*reinterpret_cast<int *>(0L) = 42; /*SIGSEGVs*/								\
	exit(1);																	\
	};

#endif