#include "component.h"

#include "event.h"

io::json component_base_t::to_json() const {
	return io::json::merge_objects(
		clock, event_counts.to_json(), model.to_json());
}

void component_base_t::account(event_base_t *event) {
	if(event_counts.find(event->get_name()) == event_counts.end()) {
		event_counts.insert(event->get_name(), counter_stat_t<uint32_t>());
		event_counts[event->get_name()].reset();
		event_counts[event->get_name()].inc();
		return;
	}
	event_counts[event->get_name()].inc();
}

void component_base_t::track(std::string key) {
	model[key] = model_stat_t(key);
	double dynamic = 0., steady = 0., leakage = 0.;
	size_t zero_idx = 0;
	warn_msg(config["power"][key][zero_idx] != nullptr, 
		"No dynamic power provided for %s", key.c_str());
	warn_msg(config["power"][key][1] != nullptr, 
		"No steady power provided for %s", key.c_str());
	warn_msg(config["power"][key][2] != nullptr, 
		"No leakage power provided for %s", key.c_str());
	JSON_CHECK(double, config["power"][key][zero_idx], dynamic);
	JSON_CHECK(double, config["power"][key][1], steady);
	JSON_CHECK(double, config["power"][key][2], leakage);

	model[key].set_power(dynamic, steady, leakage);
}

component_base_t::power_t component_base_t::get_power() {
	power_t total;
	for(auto p : model) {
		auto ps = p.second.get_power();
		total.leakage += ps.leakage;
		total.steady += ps.steady;
		total.dynamic += ps.dynamic;
	}
	return total;
}

io::json component_base_t::model_stat_t::to_json() const {
	std::map<std::string, double> p = {
		{"count", running.get() + total.get()},
		{"leakage", power.leakage},
		{"steady", power.steady},
		{"dynamic", power.dynamic * (running.get() + total.get())}
	};
	return io::json{name : p};
}