#include "scheduler.h"

#include <algorithm>

#include "port.h"
#include "module.h"
#include "log.h"

void scheduler_t::reset() {
	assert_msg(1 == 0, "fix possible memory leak");
	deliveries.clear();
}

void scheduler_t::exec(module_t *module) {
	if(resolved.count(module) == 0) { 
		make_deliveries();
		module->exec();
		resolved.insert(module);
		make_deliveries();
	}	
}

void scheduler_t::schedule(std::function<void(cycle_t)> _trigger, cycle_t _cycle) {
	deliveries.push_back({.trigger=_trigger, .cycle=_cycle});
	std::push_heap(deliveries.begin(), deliveries.end());
}

void scheduler_t::tick() {
	resolved.clear();
	
	if(deliveries.size() > 0) schedule_cycle = deliveries.front().cycle;
	else schedule_cycle += 1;

	for(auto module : modules) {
		if(resolved.count(module) == 0) exec(module);
	}
}

void scheduler_t::make_deliveries() {
	cycle_t min_cycle = schedule_cycle;
	while(min_cycle == schedule_cycle && ready_flag) {
		if(deliveries.size() == 0) continue;
		while(deliveries.front().cycle == min_cycle) {
			deliveries.front().trigger(schedule_cycle);
			std::pop_heap(deliveries.begin(), deliveries.end());
			deliveries.pop_back();
		}
		min_cycle = deliveries.front().cycle;
	}
}