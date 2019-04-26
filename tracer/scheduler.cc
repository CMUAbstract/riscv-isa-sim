#include "scheduler.h"

#include <algorithm>

#include "module.h"
#include "log.h"

void scheduler_t::reset() {
	assert_msg(1 == 0, "fix possible memory leak");
	deliveries.clear(); // Could lead to possible memory leak if events are plain pointers
}

void scheduler_t::schedule(std::function<void()> _trigger, cycle_t _cycle) {
	deliveries.push_back({.trigger=_trigger, .cycle=_cycle});
	std::push_heap(deliveries.begin(), deliveries.end());
}

void scheduler_t::tick() {
	cycle_t min_cycle = deliveries.front().cycle;
	cycle_t old_min_cycle = min_cycle;
	while(min_cycle == old_min_cycle && ready_flag) {
		while(deliveries.front().cycle == min_cycle) {
			deliveries.front().trigger();
			std::pop_heap(deliveries.begin(), deliveries.end());
			deliveries.pop_back();
		}
		for(auto action : actions) action->exec();
		min_cycle = deliveries.front().cycle;
	}
}