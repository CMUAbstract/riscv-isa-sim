#ifndef PENDING_HANDLER
#define PENDING_HANDLER

#include <vector>

#define CHECK_PENDING(event) { check_pending(event); }
#define CHECK_VOID_PENDING { check_pending(); }

class pending_event_t;
class pending_handler_t {
public:
	virtual void process(pending_event_t *event) = 0;
protected:
	void register_pending(pending_event_t *event) {
		pending_events.push_back(event);
	}
	template<class T>
	void check_pending(T event);
	void check_pending();
private:
	std::vector<pending_event_t *> pending_events;
};

#endif