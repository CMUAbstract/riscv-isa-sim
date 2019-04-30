#ifndef STAT_H
#define STAT_H

#include <sstream>
#include <array>
#include <string>
#include <vector>
#include <map>

#include <io/io.h>

class stat_t: public io::serializable {
public:
	stat_t() {}
	stat_t(std::string _name) : name(_name) {}
	stat_t(std::string _name, std::string _desc) : name(_name), desc(_desc) {}
	virtual ~stat_t() {}
	void set_name(std::string _name) { name = _name; }
	void set_desc(std::string _desc) { desc = _desc; }
	std::string get_name(void) const { return name; }
	std::string get_desc(void) const { return desc; }
protected:
	std::string name;
	std::string desc;
};

template <typename T = stat_t *>
class list_stat_t : public stat_t {
public:
	using stat_t::stat_t;
	void push_back(T s) { list.push_back(s); }
	T get(size_t idx) const { return list[idx]; }
	void update(T s) { push_back(s); }
	typename std::vector<T>::iterator begin() { return list.begin(); }
	typename std::vector<T>::iterator end() { return list.end(); }
	io::json to_json() const {
		if(name.size() > 0) return io::json::object{{name, list}};
		return io::json(list);
	}
	void integrate(const list_stat_t<T>& s) {
		list.insert(list.end(), s.begin(), s.end());
	}
protected:
	std::vector<T> list;
};

template<typename Key, typename Value = stat_t *>
class map_stat_t : public stat_t {
public:
	map_stat_t(): map_stat_t("", "") {}
	map_stat_t(std::string _name) : map_stat_t(_name, "") {}
	map_stat_t(std::string _name, std::string _desc) : stat_t(_name, _desc) {}
	map_stat_t(std::string _name, const std::map<Key, Value>&_m)
		: stat_t(_name), m(_m) {} 
	map_stat_t(std::string _name, std::string _desc, const std::map<Key, Value>&_m)
		: stat_t(_name, _desc), m(_m) {} 
	stat_t* get(Key key) const { return m[key]; }
	void insert(Key key, Value s) { m.insert(std::pair<Key, Value>(key, s)); }
	void update(std::pair<Key, Value> s) { m.insert(s); }
	typename std::map<Key, Value>::iterator find(Key key) { return m.find(key); }
	typename std::map<Key, Value>::iterator begin() { return m.begin(); }
	typename std::map<Key, Value>::iterator end() { return m.end(); }
	Value& operator[](const Key& k) { return m[k]; }
	io::json to_json() const {
		std::map<std::string, Value> sm;
		for(auto it : m) {
			std::ostringstream os;
			os << it.first;
			sm.insert(std::make_pair(os.str(), it.second));
		}
		if(name.size() > 0) return io::json::object{{name, sm}};
		return io::json(sm);
	}
	void integrate(const map_stat_t<Key, Value>& s) {
		m.insert(s.begin(), s.end());
	}
private:
	std::map<Key, Value> m;
};

template<typename T>
class scalar_stat_t : public stat_t {
public:
	using stat_t::stat_t;
	T get(void) const { return val; }
	void set(T v) { val = v; }
	void update(T v) { set(v); }
	io::json to_json() const {
		if(name.size() > 0) return io::json::object{{name, val}};
		return io::json(val);
	}
	void integrate(const scalar_stat_t<T>& s) { set(s.get()); }
protected:
	T val;
};

template<typename T, size_t size>
class array_stat_t : public stat_t {
public:
	using stat_t::stat_t;
	T get(size_t idx) const { return vals[idx]; }
	void set(T val, size_t idx) { vals[idx] = val; }
	void set(std::array<T, size>& _vals) { vals = _vals; }
	void update(std::array<T, size>& _vals) { set(_vals); }
	io::json to_json() const {
		if(name.size() > 0) return io::json::object{{name, vals}};
		return io::json(vals);
	}
protected:
	std::array<T, size> vals;
};

template<typename T>
class vector_stat_t : public stat_t {
public:
	using stat_t::stat_t;
	void push_back(T v) { vals.push_back(v); }
	void update(T v) { push_back(v); }
	T get(size_t idx) const { return vals[idx]; }
	std::vector<T>& get(void) { return vals; }
	io::json to_json() const {
		if(name.size() > 0) return io::json::object{{name, vals}};
		return io::json(vals);
	}
	void integrate(const vector_stat_t<T>& s) { 
		inc(s.get()); 
		vals.insert(vals.end(), s.get().begin(), s.get().end());
	}
protected:
	std::vector<T> vals;
};

template<typename T>
class counter_stat_t : public scalar_stat_t<T> {
public:
	using scalar_stat_t<T>::scalar_stat_t;
	void inc(T v) { this->val += v;}
	void inc(void) { this->val++; }
	void update(T v) { inc(v); }
	void reset(void) { this->val = 0; }
	void integrate(const counter_stat_t<T>& s) { inc(s.get()); }
};

template<typename T>
class running_stat_t : public stat_t {
public:
	running_stat_t() : running_stat_t("", "") {}
	running_stat_t(std::string _name) : running_stat_t(_name, "") {}
	running_stat_t(std::string _name, std::string _desc) : stat_t(_name, _desc) {
		running.reset();
		overall.reset();
	}

	template<class K, class V>
	K get(V idx) { return running.get(idx); }
	template<class K>
	K get() { return running.get(); }
	template<class K>
	void update(K& v) { running.update(v); }
	
	T total() {
		T f;
		f.reset();
		f.integrate(running);
		f.integrate(overall);
		return f;
	}
	io::json to_json() const {
		T f(name, desc);
		f.reset();
		f.integrate(running);
		f.integrate(overall);
		return f.to_json();
	}
	void reset() {
		overall.integrate(running);
		running.reset(); 
	}
public:
	T running;
	T overall;
};

#endif
