#ifndef SIGNAL_H
#define SIGNAL_H

#include "log.h"

template <typename T>
struct signal_event_t;
struct ready_event_t;
struct stall_event_t;
class signal_handler_t {
public:
	virtual void process(stall_event_t *event) = 0;
	virtual void process(ready_event_t *event) = 0;
	template <typename T>
	void process(signal_event_t<T> *event) {
		assert_msg(1 == 0, "generic signal event handler invoked");
	}
};

#endif