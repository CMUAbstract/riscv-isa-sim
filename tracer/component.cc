#include "component.h"

#include <experimental/array>

#include "event.h"

void component_base_t::reset(reset_level_t level) {
	clock.reset();
	for(auto &c : count) c.second.reset();
}

io::json component_base_t::to_json() const {
	return io::json::merge_objects(clock, event_counts.to_json(), 
		power.to_json(), energy.to_json(), count.to_json());
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

void component_base_t::track_power(std::string key) {
	warn_msg(config["power"][key] != nullptr, 
		"%s: No power provided for %s", name.c_str(), key.c_str());
	double on_power = 0., brown_power = 0.;
	JSON_CHECK(double, config["power"][key]["on"], on_power);
	JSON_CHECK(double, config["power"][key]["brown"], brown_power);
	power[key] = power_stat_t();
	std::array<double, 2> p = {on_power, brown_power};
	power[key].set(p);
}

void component_base_t::track_energy(std::string key) {
	warn_msg(config["energy"][key] != nullptr, 
		"%s: No energy provided for %s", name.c_str(), key.c_str());
	double stat_energy = 0.;
	JSON_CHECK(double, config["energy"][key], stat_energy);

	energy[key] = energy_stat_t();
	energy[key].set(stat_energy, 0.);
	count[key] = running_stat_t<counter_stat_t<uint64_t>>();
}

double component_base_t::get_power(component_base_t::power_state_t state) {
	double total = 0.;
	uint16_t which = 0;
	if(state == BROWN) which = 1;
	for(auto p : power) total += p.second.get(which);
	return total;
}

double component_base_t::get_energy() {
	double total = 0.;
	for(auto &e : energy) {
		double dynamic = e.second.get(0) * count[e.first].running.get();
		total += dynamic;
		e.second.inc(dynamic);
	}
	return total;
}

io::json component_base_t::power_stat_t::to_json() const {
	std::map<std::string, double> p = {
		{"on", vals[0]},
		{"brown", vals[1]}
	};
	return io::json{name : p};
}

io::json component_base_t::energy_stat_t::to_json() const {
	return io::json{name : io::json::merge_objects(per, total)};
}

double component_base_t::energy_stat_t::get(size_t idx) {
	if(idx == 0) return per.get();
	return total.running.get();
}