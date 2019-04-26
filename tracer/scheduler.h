#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>
#include <functional>

#include <common/decode.h>

struct action_t;
class scheduler_t {
public:
	scheduler_t() {}
	void reset();
	void set_ready() { ready_flag = true; }
	void set_ready(bool flag) { ready_flag = flag; }
	bool ready() { return ready_flag; }
	bool empty() { return deliveries.empty(); }
	void tick();
	void schedule(std::function<void()> _trigger, cycle_t _cycle);
	void register_action(action_t *_action) { actions.push_back(_action); }
private:
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