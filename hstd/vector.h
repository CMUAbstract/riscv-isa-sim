#ifndef HVEC_H
#define HVEC_H

#include <vector>
#include <unordered_map>
#include <functional>

namespace hstd {

class vector {
public:
	~vector() { clear(); }
	template<class T>
	void push_back(T _t) {
		if (items<T>.find(this) == std::end(items<T>)) {   
			clear_functions.emplace_back([](vector *_c){
				if(items<T>.find(_c) == items<T>.end()) return;
				items<T>.erase(_c);
			});
			// if someone copies me, they need to call each copy_function 
			// and pass themself
			copy_functions.emplace_back([](const vector *_from, vector *_to) {
				if(items<T>.find(_to) == items<T>.end()) return;
				if(items<T>.find(_from) == items<T>.end()) return;
				items<T>[_to] = items<T>[_from];
			});
			size_functions.emplace_back([](const vector *_c){
				if(items<T>.find(_c) == items<T>.end()) return (size_t)0;
				return items<T>[_c].size();
			});
		}
		items<T>[this].push_back(_t);
	}
	
	template<class T>
	size_t size() { return items<T>[this].size(); }
	size_t size() {
		size_t sum = 0;
		for(auto&& size_func : size_functions) {
			sum += size_func(this);
		}
		// gotta be careful about this overflowing
		return sum;
	}

	template<class T>
	typename std::vector<T>::iterator erase(typename std::vector<T>::iterator _t) { 
		return items<T>[this].erase(_t);
	}
	template<class T>
	typename std::vector<T>::iterator erase(typename std::vector<T>::iterator _start, 
		typename std::vector<T>::iterator _end) {
		return items<T>[this].erase(_start, _end); 
	}
	void clear() {
		for (auto&& clear_func : clear_functions) clear_func(this);
	}

	vector& operator=(const vector& _other) {
		clear();
		clear_functions = _other.clear_functions;
		copy_functions = _other.copy_functions;
		size_functions = _other.size_functions;
		for (auto&& copy_function : copy_functions) {
			copy_function(&_other, this);
		}
		return *this;
	}
	template<class T>
	const T& operator[](const size_t idx) const {
		return items<T>[this][idx];
	}
	template<class T>
    T& operator[](const size_t idx) {
    	return items<T>[this][idx];
    }

	template<class T>
	std::vector<T>& raw() { return items<T>[this]; }

	template<class T>
	typename std::vector<T>::iterator begin() { return items<T>[this].begin(); }
	template<class T>
	typename std::vector<T>::iterator end() { return items<T>[this].end(); } 
protected:
	std::vector<std::function<void(vector*)>> clear_functions;
	std::vector<std::function<void(const vector*, vector*)>> copy_functions;
	std::vector<std::function<size_t(const vector*)>> size_functions;
	template<class T> 
	static std::unordered_map<const vector *, std::vector<T>> items;
};

template<class T>
std::unordered_map<const vector *, std::vector<T>> vector::items;

}

#endif