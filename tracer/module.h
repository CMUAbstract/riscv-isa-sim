#ifndef MODULE_H
#define MODULE_H

#include <set>
#include <string>
#include <vector>

#include <io/io.h>
#include <stat/stat.h>

#include "port.h"
#include "scheduler.h"

struct action_t;
class module_t : public io::serializable {
public:
	module_t(std::string _name, io::json _config, scheduler_t *_scheduler);
	virtual ~module_t();

	virtual void reset();
	virtual io::json to_json() const;

	port_base_t* operator[](const std::string& key) { return ports[key]; }
	void add_port(const std::string& _port, port_base_t *other);
	port_base_t* get(const std::string& key) { return ports[key]; }
	bool has(const std::string& key) { return ports.find(key) != ports.end(); }
	
	std::string get_name() { return name; }
	virtual io::json get_config() { return config; }
	virtual std::string to_string() { return get_name(); }
	cycle_t get_clock() { return clock.get(); }
	void set_clock(cycle_t cycle) { clock.set(cycle); }

	virtual void exec() {}

	double get_static_power();
	double get_dynamic_power(uint64_t freq);
	double get_static_energy(uint64_t cycles, uint64_t freq);
	double get_dynamic_energy();

protected:
	void track_power(std::string key);
	void track_energy(std::string key);

	template<typename T>
	port_t<T> *create_port(const std::string& _port) { 
		std::string port = name + "::" + _port;
		port_t<T> *tmp = new port_t<T>(port, this, scheduler);
		ports.insert({_port, tmp}); 
		return tmp;
	}

protected:
	std::string name;
	io::json config;
	scheduler_t *scheduler;
	std::map<std::string, port_base_t *> ports;

protected:
	class work_stat_t : public stat_t {
	public:
		work_stat_t() {}
		work_stat_t(const std::string& _name, double _per) 
			: stat_t(_name), count("count"), total("total"), per(_per) {
			reset();
		}

		io::json to_json() const;
		
		void reset() {
			total.reset();
			count.reset();
		}
		void set(double t) { total.running.set(t); }
		void inc(double t) { 
			count.running.inc();
			total.running.inc(per);
		}
		double get() { return total.running.get(); }
		double get_per() { return per; }

	private:
		running_stat_t<counter_stat_t<uint64_t>> count;
		running_stat_t<counter_stat_t<double>> total;
		double per;
	};
	counter_stat_t<cycle_t> clock;
	map_stat_t<std::string, work_stat_t> power;
	map_stat_t<std::string, work_stat_t> energy;
};

class composite_t : public module_t {
public:
	composite_t(std::string _name, io::json _config, scheduler_t *_scheduler);
	~composite_t() { for(auto it : modules) delete it.second; }
	module_t * operator[](const std::string& key) { return modules[key]; }
	virtual void exec();
protected:
	std::map<std::string, module_t *> modules;
};

#endif