#ifndef EVENT_H
#define EVENT_H

#include <string>
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <typeinfo>

#include <common/decode.h>
#include <hstd/uuid.h>
#include "log.h"

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
	eventref_t() {}
	template<class T, typename std::enable_if<
		std::is_pointer<typename std::decay<T>::type>::value, int>::type = 0>
	eventref_t(T e): id(e->id) {}
	eventref_t(hstd::uuid _id): id(_id) {}
	eventref_t(eventref_t const& copy): id(copy.get_id()) {}
	eventref_t& operator=(eventref_t rhs) {
		rhs.swap(*this);
		return *this;
	}
	void swap(eventref_t& other) noexcept {
		std::swap(id,  other.id);
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
class event_heap_t {
public:
	void heapify();
	void push_back(event_base_t * e);
	event_base_t * pop_back();
	size_t size() { return events_heap.size(); }
	bool empty() { return events_heap.size() == 0; }
	
	std::vector<event_base_t *>::iterator 
	erase(std::vector<event_base_t *>::iterator it) { 
		events_set.erase((*it)->id);
		return events_heap.erase(it); 
	}
	void clear() { 
		events_heap.clear(); 
		events_set.clear();
	}
	
	std::vector<event_base_t *>::iterator begin() { return events_heap.begin(); }
	std::vector<event_base_t *>::iterator end() { return events_heap.end(); }
	void invalidate(eventref_t e) { events_set.erase(e); }
	size_t count(eventref_t e) { return events_set.count(e); }
	
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
	std::set<eventref_t> events_set;
	bool ready_flag = false;
};

template<class T>
class eventref_set_t {
public:
	class iterator: public std::map<eventref_t, T>::iterator {
	public:
		iterator(const typename std::map<eventref_t, T>::iterator &it) 
			: std::map<eventref_t, T>::iterator(it) {}
		T& operator*() { 
			return std::map<eventref_t, T>::iterator::operator*().second;
		}
		T operator->() {
			return std::map<eventref_t, T>::iterator::operator*().second;
		}
	};
public:
	size_t size() { return events_map.size(); }
	void insert(T e) { 
		events_map.insert({e, e}); 
	}
	iterator find(eventref_t e, event_heap_t *ref) {
		assert_msg(ref != nullptr, "ref event_heap is null");
		auto it = events_map.find(e);
		if(it != events_map.end() && ref->count(e) == 0) {
			events_map.erase(it); 
			return events_map.end();
		}
		return it; 
	}
	iterator erase(iterator it, event_heap_t *ref = nullptr) { 
		return events_map.erase(it); 
	}
	void clear() { events_map.clear(); }
	iterator begin(event_heap_t *ref) { 
		assert_msg(ref != nullptr, "ref event_heap is null");
		gc(ref);
		return events_map.begin(); 
	}
	iterator end(event_heap_t *ref = nullptr) { return events_map.end(); }
private:
	void gc(event_heap_t *ref) {
		typename std::map<eventref_t, T>::iterator it = events_map.begin();
		while(it != events_map.end()) {
			if(ref->count(it->first) == 0) it = events_map.erase(it);
			else ++it;
		}
	}
private:
	std::map<eventref_t, T> events_map;
};

#endif