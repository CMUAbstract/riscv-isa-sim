#ifndef HVEC_H
#define HVEC_H

#include <vector>
#include <unordered_map>
#include <functional>

class hvec {
public:
	template<class T>
	void push_back(T _t) {
		if (items<T>.find(this) == std::end(items<T>)) {   
			clear_functions.emplace_back([](hvec& _c){items<T>.erase(&_c);});
			// if someone copies me, they need to call each copy_function 
			// and pass themself
			copy_functions.emplace_back([](const hvec& _from, hvec& _to) {
				items<T>[&_to] = items<T>[&_from];
			});
			size_functions.emplace_back([](const hvec& _c){
				return items<T>[&_c].size();
			});
		}
		items<T>[this].push_back(_t);
	}
	
	template<class T>
	size_t size() { return items<T>[this].size(); }
	size_t size() {
		size_t sum = 0;
		for (auto&& size_func : size_functions) {
			sum += size_func(*this);
		}
		// gotta be careful about this overflowing
		return sum;
	}

	template<class T>
	void erase(typename std::vector<T>::iterator _t) { items<T>[this].erase(_t); }
	template<class T>
	void erase(typename std::vector<T>::iterator _start, 
		typename std::vector<T>::iterator _end) {
		items<T>[this].erase(_start, _end); 
	}
	void clear() {
		for (auto&& clear_func : clear_functions) clear_func(*this);
	}

	hvec& operator=(const hvec& _other) {
		clear();
		clear_functions = _other.clear_functions;
		copy_functions = _other.copy_functions;
		size_functions = _other.size_functions;
		for (auto&& copy_function : copy_functions) {
			copy_function(_other, *this);
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
	std::vector<std::function<void(hvec&)>> clear_functions;
	std::vector<std::function<void(const hvec&, hvec&)>> copy_functions;
	std::vector<std::function<size_t(const hvec&)>> size_functions;
	template<class T> 
	static std::unordered_map<const hvec *, std::vector<T>> items;
};

template<class T>
std::unordered_map<const hvec *, std::vector<T>> hvec::items;

#endif