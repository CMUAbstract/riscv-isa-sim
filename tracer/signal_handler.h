#ifndef SIGNAL_H
#define SIGNAL_H

#include "log.h"

struct ready_event_t;
struct stall_event_t;
class signal_handler_t {
public:
	virtual void process(stall_event_t *event) = 0;
	virtual void process(ready_event_t *event) = 0;
};

#endif