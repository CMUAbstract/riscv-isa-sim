#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <string>
#include <map>
#include <set>
#include <unordered_set>
#include <vector>
#include <functional>

#include <common/decode.h>

class module_t;
struct action_t;
class scheduler_t {
public:
	scheduler_t() {}
	~scheduler_t();
	void reset();
	void set_ready() { ready_flag = true; }
	void set_ready(bool flag) { ready_flag = flag; }
	bool ready() { return ready_flag; }
	bool empty() { return deliveries.empty(); }
	void tick();
	void schedule(std::function<void()> _trigger, cycle_t _cycle);
	void schedule_actions();
	void register_action(module_t *_module, action_t *_action,
		const std::vector<std::string>& inputs,
		const std::vector<std::string>& outputs);
private:
	struct node_t {
		action_t *action;
		std::set<node_t *> deps;
		std::unordered_set<node_t *> links;
	};
	// From port to action_node_t
	std::map<std::string, std::unordered_set<node_t *>> input_graph;
	std::map<std::string, std::unordered_set<node_t *>> output_graph;
	std::vector<node_t *> nodes;
	std::unordered_set<module_t *> modules;
	std::vector<action_t *> actions;

	struct trigger_t {
		std::function<void()> trigger;
		cycle_t cycle;
		bool operator<(trigger_t other) const {
			return cycle > other.cycle;
		}
	};
	std::vector<trigger_t> deliveries;
	bool ready_flag = true;
};

#endif