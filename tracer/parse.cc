#include "parse.h"

#include <string>
#include <set>

#include "log.h"

std::map<std::string, module_creator_t> parse_types(io::json config) {
	std::map<std::string, module_creator_t> types;
	for(auto it : config.object_items()) {
		assert_msg(it.second["type"].is_string(), "%s has no type", it.first.c_str());
		std::string type = it.second["type"].string_value();
		if(types.count(type) == 1) { // Alias-like type
			types.insert({it.first, types[type]});
		} else {
			types.insert({it.first, module_type_map.at(type)(it.second)});
		}
	}
	return types;
}

std::tuple<std::string, std::string, cycle_t> parse_cxn(std::string cxn) {
	size_t idx = cxn.find("::");
	assert_msg(idx != cxn.size(), "Invalid connection (%s)", cxn.c_str());
	std::string module = cxn.substr(0, idx);
	std::string port = cxn.substr(0, idx + 2);
	cycle_t delay = 0;
	idx = port.find("@");
	if(idx != port.size()) {
		delay = std::stoi(port.substr(idx + 1, port.size()));
		port = port.substr(0, idx);
	}
	return std::make_tuple(module, port, delay);
}

std::map<std::string, module_t *> parse_cxns(io::json config, 
	const std::map<std::string, module_creator_t>& types, scheduler_t *scheduler) {
	std::map<std::string, module_t *> modules;
	std::map<std::string, std::tuple<std::string, std::string, cycle_t>> unresolved_ports;
	
	for(auto it : config.object_items()) {
		auto from = parse_cxn(it.first);
		auto from_module = std::get<0>(from);
		
		assert_msg(it.second.is_string(), "Invalid connection (%s)", it.first.c_str());
		auto to_str = it.second.string_value();
		auto to = parse_cxn(to_str);
		auto to_module = std::get<0>(from);
		unresolved_ports.insert({it.first, from});
		unresolved_ports.insert({to_str, to});

		if(modules.count(it.first) == 0) {
			modules.insert({it.first, types.at(it.first)(it.first, scheduler)});
		}

		if(modules.count(to_str) == 0) {
			modules.insert({to_str, types.at(to_str)(to_str, scheduler)});
		}
	}

	std::set<std::string> resolved_ports;
	std::set<std::string> resolved_cxns;
	while(resolved_ports.size() < unresolved_ports.size()) {
		size_t old_size = resolved_ports.size();
		for(auto it : config.object_items()) {
			auto from = it.first;
			auto to = it.second.string_value();
			std::string from_module, from_port, to_module, to_port;
			cycle_t delay;
			std::tie(from_module, from_port, std::ignore) = unresolved_ports[from];
			std::tie(to_module, to_port, delay) = unresolved_ports[to];
			if(resolved_ports.count(to) == 0 && 
				modules[to_module]->has(to_port)) {
				resolved_ports.insert(to);
			}
			if(resolved_ports.count(from) == 0 && 
				modules[from_module]->has(from_port)) {
				resolved_ports.insert(from);
			}

			if(resolved_ports.count(to) == 1 && resolved_ports.count(from) == 0) {
				modules[from_module]->add_port(
					from_port, modules[to_module]->get(to_port));
				resolved_ports.insert(from);
			}

			std::string cxn = from + to;
			if(resolved_ports.count(from) == 1 && 
				resolved_ports.count(to) == 1 && resolved_cxns.count(cxn) == 0) {
				modules[from_module]->get(from_port)->connect(
					modules[to_module]->get(to_port), delay);
				resolved_cxns.insert(cxn);
			}
		}
		assert_msg(old_size < resolved_ports.size(), "Unresolvable connections");
	}

	return modules;
}