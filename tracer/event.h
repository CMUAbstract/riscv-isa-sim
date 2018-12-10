#ifndef EVENT_H
#define EVENT_H

#include <string>
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <typeinfo>
#include <limits>

#include <common/decode.h>
#include <hstd/uuid.h>
#include <hstd/memory.h>

#include <fesvr/memif.h>
#include <common/decode.h>

#include "log.h"
#include "component.h"

typedef uint64_t cycle_t;

struct event_base_t {
	event_base_t(cycle_t _cycle)
		: id(), cycle(_cycle), priority(std::numeric_limits<cycle_t>::max()) {}
	virtual void handle() = 0;
	virtual std::string to_string() = 0;
	virtual std::string get_name() = 0;
	virtual ~event_base_t() {}
	hstd::uuid id;
	cycle_t cycle = 0;
	cycle_t priority;
	bool ready_gc = true;
	bool squashed = false;
};

#define HANDLER_INFO 0
#if HANDLER_INFO == 2
#define HANDLER 																\
	void handle() { 															\
		auto handler = dynamic_cast<component_base_t *>(this->handler);			\
		if(handler != nullptr) {												\
			std::cout << handler->get_name();									\
			std::cout << " (" << handler->get_clock() << ")=> ";				\
		} else {																\
			std::cout << "generic";												\
			std::cout << " => ";												\
		}																		\
		std::cout << this->to_string();											\
		std::cout << " @ " << cycle;											\
		std::cout << std::endl;													\
		this->handler->process(this);											\
	}
#elif HANDLER_INFO == 1
#define HANDLER 																\
	void handle() { 															\
		auto handler = dynamic_cast<component_base_t *>(this->handler);			\
		if(handler != nullptr) {												\
			std::cout << handler->get_name() << ", ";							\
		} else {																\
			std::cout << "generic, ";											\
		}																		\
		std::cout << this->get_name();											\
		std::cout << ", " << cycle;												\
		std::cout << std::endl;													\
		this->handler->process(this);											\
	}
#else
#define HANDLER void handle() { this->handler->process(this); }
#endif

template <typename T, typename K>
struct event_t: public event_base_t {
	event_t(T *_handler, K _data)
		: event_base_t(0), handler(_handler), data(_data) {}
	event_t(T *_handler, K _data, cycle_t _cycle)
		: event_base_t(_cycle), handler(_handler), data(_data) {}
	virtual ~event_t() {}
	T* handler = nullptr;
	K data;
};

struct eventref_t {
	template<class T, typename std::enable_if<
		std::is_pointer<typename std::decay<T>::type>::value, int>::type = 0>
	eventref_t(T e): id(new hstd::uuid(e->id)) {}
	eventref_t(hstd::shared_ptr<hstd::uuid> _id): id(_id) {}
	eventref_t(eventref_t const& copy): id(copy.get_id()) {}
	eventref_t& operator=(eventref_t rhs) {
		rhs.swap(*this);
		return *this;
	}
	void swap(eventref_t& other) noexcept {
		std::swap(id,  other.id);
	}
	bool operator==(const eventref_t &other) const { 
		return *get_id() == *(other.get_id()); 
	}
	bool operator<(const eventref_t &other) const { 
		return *get_id() < *(other.get_id()); 
	}
	hstd::shared_ptr<hstd::uuid> get_id(void) const { return id; }
	hstd::shared_ptr<hstd::uuid> id;
};

// Heap map that iterates like a vector (oh man!!)
class event_heap_t {
public:
	void heapify();
	void push_back(event_base_t * e);
	event_base_t * pop_back();
	uint32_t size() { return events_heap.size(); }
	bool empty() { return events_heap.size() == 0; }
	
	std::vector<event_base_t *>::iterator 
	erase(std::vector<event_base_t *>::iterator it) { 
		events_set.erase(*it);
		return events_heap.erase(it); 
	}
	void clear() { 
		events_heap.clear(); 
		events_set.clear();
	}
	
	std::vector<event_base_t *>::iterator begin() { return events_heap.begin(); }
	std::vector<event_base_t *>::iterator end() { return events_heap.end(); }
	void invalidate(eventref_t e) { events_set.erase(e); }
	uint32_t count(eventref_t e) { return events_set.count(e); }
	
	bool ready() { return ready_flag; }
	void set_ready() { ready_flag = true; }
	void set_ready(bool val) { ready_flag = val; }	
private:
	struct comparator_t {
		bool operator()(const event_base_t *a,const event_base_t* b) const{
			if(a->cycle == b->cycle) return a->priority > b->priority;
			return a->cycle > b->cycle;
		}
	};
private:
	std::vector<event_base_t *> events_heap;
	std::set<eventref_t> events_set;
	bool ready_flag = false;
};

#endif