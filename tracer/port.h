#ifndef PORT_H
#define PORT_H

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

#include <stat/stat.h>
#include <io/io.h>

#include "event.h"
#include "scheduler.h"

class port_t : public io::serializable {
public:
	port_t(scheduler_t *_scheduler, counter_stat_t<cycle_t> *_clock)
		: port_t("", _scheduler, _clock) {}
	port_t(const std::string& _name, scheduler_t *_scheduler, 
		counter_stat_t<cycle_t> *_clock) 
		: name(_name), scheduler(_scheduler), clock(_clock) {}
	virtual ~port_t() {}

	std::string get_name() { return name; }
	virtual std::string to_string() { return name; }
	virtual io::json to_json() const { return nullptr; }

	virtual void connect(port_t *p, cycle_t delay) = 0;
	virtual port_t * clone(const std::string& _name, 
		scheduler_t *scheduler, counter_stat_t<cycle_t> *_clock) = 0;

protected:
	std::string name;
	scheduler_t *scheduler;
	counter_stat_t<cycle_t> *clock;
};

template<typename T>
class port_impl_t : public port_t {
public:
	using port_t::port_t;

	virtual void connect(port_t *p, cycle_t delay=0) {
		// I don't think it is feasible to implement without dynamic_cast
		// and it will probably be a whole lot slower too because
		// the dynamic cast allows easy conversion from string to type;
		// otherwise I would have to write a ton of code to parse the input
		T *cxn = dynamic_cast<T *>(p);
		assert_msg(cxn, 
			"Connection between %s and %s failed", to_string(), p->to_string());
		cxns.push_back({.port=cxn, delay=delay});
	}

	virtual bool connected() { return !cxns.empty(); }

protected:
	struct cxn_t {
		T *port;
		cycle_t delay;
	};
	std::vector<cxn_t> cxns;
};

template<typename T, typename Compare=event_comparator_t>
class signal_port_t : public port_impl_t<signal_port_t<T>> {
public:
	using port_impl_t<signal_port_t<T>>::port_impl_t;
	
	std::string to_string() {
		std::ostringstream os;
		os << this->name << "(" << size << ")"; 
		return os.str();
	}

	virtual T peek() { return heap.front(); }

	virtual void pop() {
		std::pop_heap(heap.begin(), heap.end(), comparator);
		heap.pop_back();
	}

	virtual void push(T e) {
		for(auto cxn : this->cxns) {
			// set tick of event and do a copy if necessary
			this->scheduler->schedule([&](){ cxn.port->accept(e); }, 
				this->clock->get() + cxn.delay);
		}
	}
	virtual void accept(T e) {
		heap.push_back(e);
		std::push_heap(heap.begin(), heap.end(), comparator);
	}

	virtual bool empty() { return heap.empty(); }
	virtual size_t size() { return heap.size(); }

	// Yay covariant return types!
	virtual signal_port_t<T>* clone(const std::string& _name, 
		scheduler_t *_scheduler, counter_stat_t<cycle_t> *_clock) {
		return new signal_port_t<T>(_name, _scheduler, _clock);
	}

protected:
	std::vector<T> heap;
	Compare comparator;
};

template<typename T>
class persistent_port_t : public port_impl_t<persistent_port_t<T>> {
public:
	using port_impl_t<signal_port_t<T>>::port_impl_t;

	virtual T set_default(T e) { data = e; }

	virtual T peek() { return data; }

	virtual void pop() {
		for(auto cxn : this->cxns) {
			// set tick of event and do a copy if necessary
			this->scheduler->schedule([&](){ cxn.port->accept(data); }, 
				this->clock->get() + 1);
		}
	}

	virtual void push(T e) {
		for(auto cxn : this->cxns) {
			// set tick of event and do a copy if necessary
			this->scheduler->schedule([&](){ cxn.port->accept(e); }, 
				this->clock->get() + cxn.delay);
		}
	}
	virtual void accept(T e) { data = e; }

	virtual bool empty() { return false; }
	virtual size_t size() { return 0; }

	// Yay covariant return types!
	virtual persistent_port_t<T> * clone(const std::string& _name, 
		scheduler_t *_scheduler, counter_stat_t<cycle_t> *_clock) {
		auto tmp = new persistent_port_t<T>(_name, _scheduler, _clock);
		tmp->set_default(data);
		return tmp;
	}

protected:
	T data;
};

#endif