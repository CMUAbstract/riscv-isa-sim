#ifndef EVENT_H
#define EVENT_H

#include <string>
#include <map>
#include <set>
#include <iostream>

#include <common/decode.h>
#include <hstd/uuid.h>

typedef uint64_t cycle_t;

struct event_base_t {
	event_base_t(cycle_t _cycle): cycle(_cycle), id() {}
	virtual void handle() = 0;
	virtual std::string to_string() = 0;
	virtual std::string get_name() = 0;
	virtual ~event_base_t() {}
	cycle_t cycle = 0;
	hstd::uuid id;
	bool ready_gc = true;
	bool pending = false;
	bool squashed = false;
};

#define HANDLER_INFO 2
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
	template<class T>
	eventref_t(T *e): id(e->id) {}
	eventref_t(hstd::uuid _id): id(_id) {}
	eventref_t& operator=(eventref_t rhs) {
		id = rhs.id;
		return *this;
	}
	bool operator==(const eventref_t &other) const { 
		return get_id() == other.get_id(); 
	}
	bool operator<(const eventref_t &other) const { 
		return get_id() < other.get_id(); 
	}
	hstd::uuid get_id(void) const { return id; }
	hstd::uuid id;
};

// Heap map that iterates like a vector (oh man!!)
class event_hmap_t {
public:
	struct iterator: public std::map<eventref_t, event_base_t *>::iterator {
		iterator(const std::map<eventref_t, event_base_t *>::iterator &it)
			: std::map<eventref_t, event_base_t *>::iterator(it) {}
		event_base_t*& operator*() {
			return std::map<eventref_t, event_base_t *>::iterator::operator*().second;
		}
		event_base_t* operator->() {
			return std::map<eventref_t, event_base_t *>::iterator::operator->()->second;
		}
	};
public:
	void heapify();
	void push_back(event_base_t * e);
	event_base_t * pop_back();
	size_t size() { return events.size(); }
	bool empty() { return events.size() == 0; }
	
	iterator erase(iterator it) { return events.erase(it); }
	void clear() { events.clear(); }
	
	iterator begin() { return events.begin(); }
	iterator end() { return events.end(); }

	iterator find(eventref_t e) { return events.find(e); }
	
	bool ready() { return ready_flag; }
	void set_ready() { ready_flag = true; }
	void set_ready(bool val) { ready_flag = val; }	
private:
	struct comparator_t {
		bool operator()(const event_base_t *a,const event_base_t* b) const{
			if(a->cycle == b->cycle) return a->pending <= b->pending;
			return a->cycle > b->cycle;
		}
	};
private:
	std::vector<event_base_t *> events_heap;
	std::map<eventref_t, event_base_t *> events_map;
	bool ready_flag = false;
};

template<class T>
class eventref_set_t {
public:
	struct iterator: public std::set<eventref_t>::iterator {
		iterator(const std::set<eventref_t>::iterator &it) 
			: std::set<eventref_t>::iterator(it) {}
        iterator& operator++() {

        }
		T& operator*() {

		}
		T operator->() {
			
		}
	};
public:
	eventref_set_t(): eventref_set_t(nullptr) {}
	eventref_set_t(event_hmap_t *_ref_events): ref_events(_ref_events) {}
	void set_ref(event_hmap_t *_ref_events) { ref_events = _ref_events; }
	size_t size() { return events.size(); }
	void insert(T e) { 
		events.insert(e); 
		events.push_back(e);
	}
	iterator find(eventref_t e) {
		auto it = events.find(e);
		if(it != events.end() && ref_events->find(e) != ref_events->end()) {
			events.erase(it); 
			return events.end();
		}
		return it; 
	}
	iterator erase(iterator it) { return events.erase(it); }
	void clear() { events.clear(); }
	iterator begin() { return events.begin(); }
	iterator end() { return events.end(); }
private:
	std::map<eventref_t, T> events;
	event_hmap_t *ref_events;
};

#endif