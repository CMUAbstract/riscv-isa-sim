#ifndef PORT_H
#define PORT_H

#include <string>
#include <sstream>
#include <queue>
#include <vector>
#include <algorithm>

#include <stat/stat.h>
#include <io/io.h>

#include "log.h"
#include "event.h"
#include "scheduler.h"

class port_base_t : public io::serializable {
public:
	port_base_t(const std::string& _name, module_t *_module, scheduler_t *_scheduler) 
		: name(_name),  module(_module), scheduler(_scheduler) {}
	virtual ~port_base_t() {}
	virtual port_base_t * clone(const std::string& _name, module_t *_module,
		scheduler_t *scheduler) = 0;

	std::string get_name() const { return name; }
	module_t *get_module() const { return module; }
	virtual std::string to_string() const { return name; }
	virtual io::json to_json() const { return nullptr; }

	virtual void connect(port_base_t *p, cycle_t delay) = 0;
	virtual void link(port_base_t *p) { links.push_back(p->get_module()); }
	
	const std::vector<module_t *>& get_links() { return links; }
	const std::vector<cycle_t>& get_delays() { return delays; }

protected:
	void eval() { 
		size_t idx = 0;
		for(auto link : links) if(delays[idx++] == 0) scheduler->exec(link); 
	}
	std::string name;
	module_t *module;
	scheduler_t *scheduler;
	std::vector<module_t *> links;
	std::vector<cycle_t> delays;
};

template<template<typename...> typename S, typename T>
class port_impl_t : public port_base_t {
public:
	using port_base_t::port_base_t;

	virtual void connect(port_base_t *p, cycle_t delay=0) {
		// I don't think it is feasible to implement without dynamic_cast
		// and it will probably be a whole lot slower too because
		// the dynamic cast allows easy conversion from string to type;
		// otherwise I would have to write a ton of code to parse the input
		S<T> *cxn = dynamic_cast<S<T> *>(p);
		assert_msg(cxn, "Connection between %s and %s failed", 
			to_string().c_str(), p->to_string().c_str());
		cxns.push_back(cxn);
		delays.push_back(delay);
		p->link(this); // Add a dependent link
	}

	virtual bool connected() { return !cxns.empty(); }
	
	virtual T peek() = 0;
	virtual void pop() = 0;
	virtual void push(T e) = 0;
	virtual void accept(T e, cycle_t cycle) = 0;
	virtual bool empty() = 0;
	virtual size_t size() = 0;

protected:
	std::vector<S<T> *> cxns;
};

template<typename T>
class port_t : public port_impl_t<port_t, T> {
public:
	using port_impl_t<port_t, T>::port_impl_t;
	
	std::string to_string() {
		std::ostringstream os;
		os << this->get_name() << "(" << this->size() << ")"; 
		return os.str();
	}

	virtual T peek() {
		this->eval();
		return data.front();
	}
	virtual void pop() {
		this->eval();
		data.pop();
	}

	virtual void push(T e) {
		size_t idx = 0;
		for(auto cxn : this->cxns) {
			this->scheduler->schedule([&](cycle_t cycle){ cxn->accept(e, cycle); }, 
				this->module->get_clock() + this->delays[idx]);
			idx++;
		}
	}
	virtual void accept(T e, cycle_t cycle) {
		assert_msg(cycle >= this->module->get_clock(), "Timing violation %s",
			this->get_name().c_str());
		this->module->set_clock(cycle);
		data.push(e);
	}
#if 0
	virtual void forward() {
		size_t idx = 0;
		for(auto cxn : this->cxns) {
			for(auto e : heap) {
				this->scheduler->schedule([&](){ cxn->accept(e); }, 
					this->module->get_clock() + this->delays[idx]);
			}
			idx++;
		}
		heap.clear();
		std::make_heap(heap.begin(), heap.end(), comparator);
	}
#endif

	virtual bool empty() {
		this->eval();
		return data.empty();
	}
	virtual size_t size() {
		this->eval();
		return data.size();
	}

	// Yay covariant return types!
	virtual port_t<T>* clone(const std::string& _name, 
		module_t *_module, scheduler_t *_scheduler) {
		return new port_t<T>(_name, _module, _scheduler);
	}

protected:
	std::queue<T> data;
};

#endif