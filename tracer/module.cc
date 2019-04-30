#include "module.h"

#include "scheduler.h"
#include "port.h"
#include "parse.h"
#include "log.h"

module_t::module_t(std::string _name, io::json _config, scheduler_t *_scheduler)
	: name(_name), config(_config), scheduler(_scheduler), 
	clock("clock"), power("power"), energy("energy") {
	clock.reset();
}

module_t::~module_t() {
	for(auto it : ports) delete it.second;
	for(auto it : actions) delete it.second;	
}

void module_t::reset() {
	clock.reset();
}

io::json module_t::to_json() const {
	map_stat_t<std::string, io::json> counts("counts");
	for(auto it : actions) counts.insert(it.first, it.second->count);
	return io::json::merge_objects(
		clock, power.to_json(), energy.to_json(), counts.to_json());
}

void module_t::register_action(action_t *action, 
	const std::vector<std::string>& inputs,
	const std::vector<std::string>& outputs) {
	scheduler->register_action(this, action, inputs, outputs);
	actions.insert({action->get_name(), action});
}

void module_t::add_port(const std::string& _port, port_t *other) {
	std::string port = name + "::" + _port;
	port_t *tmp = other->clone(port, scheduler, &clock);
	ports.insert({_port, tmp});
}

double module_t::get_static_power() {
	assert_msg(1 == 0, "Not yet reimplemented");
	return 0;
}

double module_t::get_dynamic_power(uint64_t freq) {
	assert_msg(1 == 0, "Not yet reimplemented");
	return 0;
}

double module_t::get_static_energy(uint64_t cycles, uint64_t freq) {
	assert_msg(1 == 0, "Not yet reimplemented");
	return 0;
}

double module_t::get_dynamic_energy() {
	assert_msg(1 == 0, "Not yet reimplemented");
	return 0;
}

void module_t::track_energy(std::string key) {
	warn_msg(config["energy"][key] != nullptr, 
		"%s: No energy provided for %s", name.c_str(), key.c_str());
	double stat_energy = 0.;
	JSON_CHECK(double, config["energy"][key], stat_energy);

	energy[key] = work_stat_t(key, stat_energy);
}

void module_t::track_power(std::string key) {
	warn_msg(config["power"][key] != nullptr, 
		"%s: No power provided for %s", name.c_str(), key.c_str());
	double stat_power = 0.;
	JSON_CHECK(double, config["energy"][key], stat_power);

	power[key] = work_stat_t(key, stat_power);
}

io::json module_t::work_stat_t::to_json() const {
	return io::json::merge_objects(count, total);
}

composite_t::composite_t(std::string _name, io::json _config, scheduler_t *_scheduler)
	: module_t(_name, _config, _scheduler) {
	std::cerr << "Synthesizing: " << name << std::endl;
	assert_msg(config.is_object(), "%s has no config", name.c_str());
	assert_msg(config["modules"].is_object(), "%s has no modules", name.c_str());
	assert_msg(config["cxns"].is_object(), "%s has no connections", name.c_str());
	auto types = parse_types(config["modules"]);
	modules = parse_cxns(this, config["cxns"], types, scheduler);

	for(auto port : ports) {
		register_action(new action_t(port.first, [&](){
			port.second->forward();
		}), {port.first}, {port.first});
	}
}