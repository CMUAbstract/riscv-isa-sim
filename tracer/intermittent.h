#ifndef INTERMITTENT_H
#define INTERMITTENT_H

#include <common/decode.h>

class intermittent_t {
protected:
	intermittent_t() { fail(); }
	void fail();
	bool should_fail(cycle_t cycle) { return cycle >= fail_cycle; }
protected:
	bool intermittent = false;
	uint32_t intermittent_max = 350000;
	uint32_t intermittent_min = 150000;
private:
	cycle_t fail_cycle = 0;
	bool fail_flag = false;
};

#endif