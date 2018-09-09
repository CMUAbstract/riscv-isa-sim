#ifndef STAT_H
#define STAT_H

#include <vector>
#include <string>
#include <sstream> 

class stat_t {
public:
	stat_t() {}
	stat_t(std::string _name, std::string _desc) : name(_name), desc(_desc) {}
	virtual ~stat_t() {}
	void set_name(std::string _name) { name = _name; }
	void set_desc(std::string _desc) { desc = _desc; }
	std::string get_name(void) const { return name; }
	std::string get_desc(void) const { return desc; }
	virtual std::string dump(void) const { 
		std::ostringstream os;
		os << "\"" << name << "\":" << "\"\"";
		return os.str();
	}
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
	std::string dump(void) const {
		std::ostringstream os;
		os << "\"" << name << "\": [" << std::endl;
		size_t idx = 0;
		for(const auto it : list) {
			os << it->dump();
			if(idx++ != list.size() - 1) os << ",";
			os << std::endl;
		}
		os << "]" << std::endl;
		return os.str();
	}
	typename std::vector<T>::iterator begin() { return list.begin(); }
	typename std::vector<T>::iterator end() { return list.end(); }
protected:
	std::vector<T> list;
};

template <typename Key, typename Value = stat_t *>
class map_stat_t : public stat_t {
public:
	using stat_t::stat_t;
	stat_t* get(Key key) const { return m[key]; }
	void insert(Key key, Value s) { m.insert(std::pair<Key, Value>(key, s)); }
	std::string dump(void) const {
		std::ostringstream os;
		os << "\"" << name << "\": {" << std::endl;
		size_t idx = 0;
		for(auto it : m) {
			os << "\"0x" << std::hex << it.first << "\": " << it.second->dump();
			if(idx++ != m.size() - 1) os << ",";
			os << std::endl;
		}
		os << "}" << std::endl;
		return os.str();
	}
	typename std::map<Key, Value>::iterator find(Key key) { return m.find(key); }
	typename std::map<Key, Value>::iterator begin() { return m.begin(); }
	typename std::map<Key, Value>::iterator end() { return m.end(); }
private:
	std::map<Key, Value> m;
};

template <typename T>
class scalar_stat_t : public stat_t {
public:
	using stat_t::stat_t;
	T get(void) const { return val; }
	void set(T v) { val = v;}
	std::string dump(void) const {
		std::ostringstream os;
		if(name.size() > 0)
			os << "\"" << name << "\"" << ":" << val;
		else os << val;
		return os.str();
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
	std::string dump(void) const {
		std::ostringstream os;
		os << "\"" << name << "\": " << "[";
		size_t idx = 0;
		for(auto it : vals) {
			os << *it;
			if(idx++ != vals.size() - 1) os << ",";
		}
		os << "]";
		return os.str();
	}
protected:
	std::vector<T> vals;
};

template <typename T>
class counter_stat_t : public scalar_stat_t<T> {
public:
	using scalar_stat_t<T>::scalar_stat_t;
	void set(T v) { this->val += v;}
	void inc(void) { this->val++; }
	void reset(void) { this->val = 0; }
};

#endif