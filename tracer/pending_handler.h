#ifndef PENDING_HANDLER
#define PENDING_HANDLER

class pending_event_t;
class pending_handler_t {
public:
	virtual void process(pending_event_t *event) = 0;
};

#endif