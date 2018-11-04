#ifndef SIGNAL_EVENT_H
#define SIGNAL_EVENT_H

#include <sstream>

#include "event.h"
#include "signal.h"

template <typename T>
struct signal_event_t: public event_t<signal_handler_t, T> {
	using event_t<signal_handler_t, T>::event_t;
};

struct ready_event_t: public signal_event_t<addr_t> {
	using signal_event_t<addr_t>::signal_event_t;
	std::string to_string() {
		std::ostringstream os;
		os << "mem_ready_event (0x" << std::hex << this->data << ")"; 
		return os.str();
	}
	HANDLER;
};

struct stall_event_t: public signal_event_t<addr_t> {
	using signal_event_t<addr_t>::signal_event_t;
	std::string to_string() {
		std::ostringstream os;
		os << "mem_stall_event (0x" << std::hex << this->data << ")"; 
		return os.str();
	}
	HANDLER;
};

#endif