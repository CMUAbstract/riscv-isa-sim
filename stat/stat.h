#ifndef STAT_H
#define STAT_H

#include <string>
#include <vector>
#include <map>

#include <io/io.h>

class stat_t {
public:
	stat_t() {}
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
	void push_back(T s){ list.push_back(s); }
	T get(size_t idx) const { return list[idx]; }
	typename std::vector<T>::iterator begin() { return list.begin(); }
	typename std::vector<T>::iterator end() { return list.end(); }
	io::json to_json() const {
		if(name.size() > 0) return io::json::object{{name, list}};
		return io::json(list);
	}
protected:
	std::vector<T> list;
};

template <typename Key, typename Value = stat_t *>
class map_stat_t : public stat_t {
public:
	using stat_t::stat_t;
	stat_t* get(Key key) const { return m[key]; }
	void insert(Key key, Value s) { m.insert(std::pair<Key, Value>(key, s)); }
	typename std::map<Key, Value>::iterator find(Key key) { return m.find(key); }
	typename std::map<Key, Value>::iterator begin() { return m.begin(); }
	typename std::map<Key, Value>::iterator end() { return m.end(); }
	io::json to_json() const {
		std::map<std::string, Value> sm;
		for(auto it : m) 
			sm.insert(std::make_pair(std::to_string(it.first), it.second));
		if(name.size() > 0) return io::json::object{{name, sm}};
		return io::json(sm);
	}
private:
	std::map<Key, Value> m;
};

template <typename T>
class scalar_stat_t : public stat_t {
public:
	using stat_t::stat_t;
	T get(void) const { return val; }
	void set(T v) { val = v;}
	io::json to_json() const {
		if(name.size() > 0) return io::json::object{{name, val}};
		return io::json(val);
	}
protected:
	T val;
};

template <typename T>
class vector_stat_t : public stat_t {
public:
	using stat_t::stat_t;
	void push_back(T v) { vals.push_back(v); }
	T get(size_t idx) const { return vals[idx]; }
	std::vector<T> get(void) { return vals; }
	io::json to_json() const {
		if(name.size() > 0) return io::json::object{{name, vals}};
		return io::json(vals);
	}
protected:
	std::vector<T> vals;
};

template <typename T>
class counter_stat_t : public scalar_stat_t<T> {
public:
	using scalar_stat_t<T>::scalar_stat_t;
	void inc(T v) { this->val += v;}
	void inc(void) { this->val++; }
	void reset(void) { this->val = 0; }
};

#endif
