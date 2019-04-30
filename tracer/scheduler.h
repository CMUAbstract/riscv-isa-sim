#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <string>
#include <set>
#include <vector>
#include <unordered_set>
#include <functional>

#include <common/decode.h>

class module_t;
class scheduler_t {
public:
	scheduler_t() {}
	void reset();
	void set_debug(bool flag=true) { debug_flag = flag; }
	void set_ready(bool flag=true) { ready_flag = flag; }
	bool ready() { return ready_flag; }
	bool debug() { return debug_flag; }
	void tick();
	void exec(module_t *module);
	void schedule(std::function<void(cycle_t)> _trigger, cycle_t _cycle);
	void register_module(module_t *module) { modules.insert(module); }
private:
	void make_deliveries();
private:
	std::unordered_set<module_t *> modules;
	std::set<module_t *> resolved;
	cycle_t schedule_cycle = -1;

	struct event_t {
		std::function<void(cycle_t)> trigger;
		cycle_t cycle;
		bool operator<(event_t other) const {
			return cycle > other.cycle;
		}
	};
	std::vector<event_t> deliveries;
	bool ready_flag = true;
	bool debug_flag = false;
};

#endif