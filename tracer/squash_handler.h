#ifndef SQUASH_HANDLER_H
#define SQUASH_HANDLER_H

#include "event.h"

struct squash_event_t;
class squash_handler_t{
public:
	virtual void process(squash_event_t *event) = 0;
protected:
	eventref_set_t<event_base_t *> squashed_events;
};

#endif