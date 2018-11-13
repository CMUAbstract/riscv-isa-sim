#ifndef HMAP_H
#define HMAP_H

#include <vector>
#include <unordered_map>
#include <map>
#include <functional>

template<class K>
class hmap {
public:
	template<class T>
	void insert(std::pair<K, T> _t) {
		if (items<T>.find(this) == std::end(items<T>)) {   
			clear_functions.emplace_back([](hmap& _c){items<T>.erase(&_c);});
			// if someone copies me, they need to call each copy_function 
			// and pass themself
			copy_functions.emplace_back([](const hmap& _from, hmap& _to) {
				items<T>[&_to] = items<T>[&_from];
			});
			size_functions.emplace_back([](const hmap& _c){
				return items<T>[&_c].size();
			});
		}
		items<T>[this].insert(_t);
	}
	template<class T>
	void insert(const K _k, T _t) { insert(std::pair<K, T>(_k, _t)); }
	
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
	void erase(typename std::map<K, T>::iterator _t) { items<T>[this].erase(_t); }
	template<class T>
	void erase(typename std::map<K, T>::iterator _start, 
		typename std::map<K, T>::iterator _end) {
		items<T>[this].erase(_start, _end); 
	}
	void clear() {
		for (auto&& clear_func : clear_functions) clear_func(*this);
	}

	hmap& operator=(const hmap& _other) {
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
	const T& operator[](const K& idx) const {
		return items<T>[this][idx];
	}
	template<class T>
    T& operator[](const K& idx) { return items<T>[this][idx]; }

	template<class T>
	std::map<K, T>& raw() { return items<T>[this]; }

	template<class T>
	typename std::map<K, T>::iterator begin() { return items<T>[this].begin(); }
	template<class T>
	typename std::map<K, T>::iterator end() { return items<T>[this].end(); } 
protected:
	std::vector<std::function<void(hmap&)>> clear_functions;
	std::vector<std::function<void(const hmap&, hmap&)>> copy_functions;
	std::vector<std::function<size_t(const hmap&)>> size_functions;
	template<class T> 
	static std::unordered_map<const hmap *, std::map<K, T>> items;
};

template<class K>
template<class T>
std::unordered_map<const hmap<K> *, std::map<K, T>> hmap<K>::items;

#endif