#include "scheduler.h"

#include <algorithm>

#include "port.h"
#include "module.h"
#include "log.h"

scheduler_t::~scheduler_t() {
	for(auto it : nodes) delete it;
}

void scheduler_t::reset() {
	assert_msg(1 == 0, "fix possible memory leak");
	deliveries.clear(); // Could lead to possible memory leak if events are plain pointers
}

void scheduler_t::schedule(std::function<void()> _trigger, cycle_t _cycle) {
	deliveries.push_back({.trigger=_trigger, .cycle=_cycle});
	std::push_heap(deliveries.begin(), deliveries.end());
}

void scheduler_t::schedule_actions() {
	actions.clear();
	// First generate dependencies
	for(auto module : modules) {
		auto cxn = module->pbegin();
		while(cxn != module->pend()) {
			size_t idx = 0;
			auto delays = cxn->second->get_delays();
			for(auto link : cxn->second->get_links()) {
				if(delays[idx] == 0) {
					auto from = output_graph.find(cxn->second->get_name());
					assert_msg(from != output_graph.end(), "%s not annotated",
						cxn->second->get_name().c_str());

					auto to = input_graph.find(link);
					assert_msg(to != input_graph.end(), "%s not annotated", 
						link.c_str());
					
					for(auto from_node : from->second) {
						for(auto to_node : to->second) {
							from_node->links.insert(to_node);
							to_node->deps.insert(from_node);
						}
					}
				}
				++idx;
			}
			++cxn;
		}
	}

	// Now flatten these dependencies out and schedule
	while(actions.size() < nodes.size()) {
		size_t old_size = actions.size();
		for(auto node : nodes) {
			if(node->deps.size() == 0) {
				actions.push_back(node->action);
				// Remove dependencies
				for(auto link : node->links) {
					link->deps.erase(node);
				}
			}
		}
		assert_msg(old_size < actions.size(), 
			"Unscheduable subcycle dependencies");
	}
}

void scheduler_t::register_action(module_t *_module, action_t *_action,
		const std::vector<std::string>& inputs,
		const std::vector<std::string>& outputs) {
	modules.insert(_module);
	node_t *node = new node_t{.action=_action};
	nodes.push_back(node);
	
	for(auto it : inputs) {
		std::string port = _module->get_name() + "::" + it;
		auto f = input_graph.find(port);
		if(f != input_graph.end()) {
			f->second.insert(node);
		} else {
			std::unordered_set<node_t *> tmp;
			tmp.insert(node);
			input_graph.insert({port, tmp});
		}
	}

	for(auto it : outputs) {
		std::string port = _module->get_name() + "::" + it;
		auto f = output_graph.find(port);
		if(f != output_graph.end()) {
			f->second.insert(node);
		} else {
			std::unordered_set<node_t *> tmp;
			tmp.insert(node);
			output_graph.insert({port, tmp});
		}
	}
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