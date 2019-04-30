#include "vec1dmulti.h"

#include "mem_value.h"
#include "vector_value.h"
#include "pending_value.h"

vec1dmulti_t::vec1dmulti_t(std::string _name, io::json _config, 
	value_heap_t *_values) : vcu_t(_name, _config, _values), retire_insn(nullptr) {
	JSON_CHECK(int, config["window_size"], window_size);
	assert_msg(window_size > 0, "Window size must be greater than zero");
	JSON_CHECK(int, config["rf_ports"], rf_ports);
	assert_msg(rf_ports > 0, 
		"Number of register file ports must be greater than zero");
}

io::json vec1dmulti_t::to_json() const {
	return vcu_t::to_json();
}

void vec1dmulti_t::reset(reset_level_t level) {
	vcu_t::reset(level);
	idx = 0;
	pe_state = READ;
	active_window_size = 0;
	active_insn_offset = 0;
	active_reg_reads = 0;
	active_reg_writes = 0;
	progress = 0;
	write_set.clear();
	read_set.clear();
}

void vec1dmulti_t::process(vec_issue_value_tvalue) {
	TIME_VIOLATION_CHECK;
	if(promote_pending(value, [&](){
		return !(active_window_size < window_size);
	}) != nullptr) return;
	
	empty = false;
	value->data->idx = idx;
	idx++;
	active_window_size++;

	// Add to table here
	for(auto it : value->data->ws.input.vregs) {
		uint8_t reg = strip_killed(it);
		bool found = write_set.find(reg) != write_set.end();
		if(!found) read_set.insert(reg);
		else if(check_killed(it) && found) write_set.erase(reg);
	}

	for(auto it : value->data->ws.output.vregs) {
		uint8_t reg = strip_killed(it);
		write_set.insert(reg);
	}

	if(active_window_size == window_size) {
		vcu_t::set_core_stage("exec", true);
		values->push_back(new vec_start_value_t(this, false, clock.get()));
	}

	auto pending_value = new pending_value_t(this, 
		new pe_exec_value_t(this, value->data, clock.get()), clock.get() + 1);
	pending_value->add_dep<vec_start_value_t>([](vec_start_value_te) {
		return true;
	});
	pending_value->add_dep([&](){ return pe_state == EXEC; });
	register_pending(pending_value);
	values->push_back(pending_value);

	if(active_window_size == 1) {
		auto pending_read_value = new pending_value_t(this, 
			new pe_read_value_t(this, &read_set, clock.get()), clock.get() + 1);
		pending_read_value->add_dep<vec_start_value_t>(
			[](vec_start_value_te) { return true; });
		register_pending(pending_read_value);
		values->push_back(pending_read_value);

		auto pending_write_value = new pending_value_t(this, 
			new pe_write_value_t(this, &write_set, clock.get()), clock.get() + 1);
		pending_write_value->add_dep<vec_start_value_t>(
			[](vec_start_value_te) { return true; });
		pending_write_value->add_dep([&](){ return pe_state == WRITE; });
		register_pending(pending_write_value);
		values->push_back(pending_write_value);
	}
	retire_insn = value->data;
}

void vec1dmulti_t::process(pe_read_value_tvalue) {
	TIME_VIOLATION_CHECK
	uint16_t work = read_set.size() - active_reg_reads;
	if(work > rf_ports) work = rf_ports;
	pending_value_tpending_value;

	if(active_reg_writes + work < read_set.size()) {
		 pending_value = new pending_value_t(this, 
			new pe_read_value_t(this, value->data), clock.get() + 1);
		pending_value->add_fini([&, work](){ active_reg_reads += work; });
	} else if(progress + lanes < vl) {
		pe_state = EXEC;
		active_reg_reads = 0;
		pending_value = new pending_value_t(this, 
			new pe_read_value_t(this, value->data), clock.get() + 1);
		pending_value->add_dep([&](){ return pe_state == READ; });
	} else {
		pe_state = EXEC;
		active_reg_reads = 0;
		pending_value = new pending_value_t(this, nullptr, clock.get() + 1);
	}

	auto it = std::next(read_set.begin(), active_reg_reads);
	auto end = std::next(read_set.begin(), active_reg_reads + work);
	while(it != end) {
		auto reg = *it;
		values->push_back(new vec_reg_read_value_t(
			this, {.reg=reg, .idx=0}, clock.get()));
		pending_value->add_dep<vec_reg_read_value_t>(
			[reg](vec_reg_read_value_te){
				return e->data.reg == reg;
		});
		++it;
	}
	register_pending(pending_value);
	values->push_back(pending_value);
}

void vec1dmulti_t::process(pe_exec_value_tvalue) {
	TIME_VIOLATION_CHECK
	// Promote to pending
	uint16_t insn_idx = value->data->idx;
	if(promote_pending(value, [&, insn_idx]() {
		return !(active_insn_offset == insn_idx);
	}) != nullptr) return;

	uint16_t work = vl - progress;
	if(work > lanes) work = lanes;

	pending_value_tpending_value;
	if(progress + lanes < vl){
		pending_value = new pending_value_t(this, 
			new pe_exec_value_t(this, value->data), clock.get() + 1);
		pending_value->add_dep([&, cur_progess=progress]() { 
			return pe_state == EXEC && cur_progess < progress; 
		});
	} else {
		pending_value = new pending_value_t(this, nullptr, clock.get() + 1);
	}

	// THIS IS SORT OF WRONG
	active_insn_offset++;
	if(insn_idx == active_window_size - 1) {
		pe_state = WRITE;
		active_insn_offset = 0;
	}

	// Input locations
	if(progress < value->data->ws.input.locs.size()) {
		auto it = std::next(value->data->ws.input.locs.begin(), progress);
		auto end = std::next(
			value->data->ws.input.locs.begin(), progress + work);
		while(it != end) {
			for(auto child : children.raw<ram_handler_t *>()) {
				auto loc = *it;
				values->push_back(
					new mem_read_value_t(child.second, loc, clock.get()));
				pending_value->add_dep<mem_retire_value_t>(
					[loc](mem_retire_value_te){
					return e->data.addr == loc;
				});
			}
			++it;
		}
	}

	// Output locations
	if(progress < value->data->ws.output.locs.size()) {
		auto it = std::next(value->data->ws.output.locs.begin(), progress);
		auto end = std::next(
			value->data->ws.output.locs.begin(), progress + work);
		while(it != end) {
			for(auto child : children.raw<ram_handler_t *>()) {
				auto loc = *it;
				values->push_back(
					new mem_read_value_t(child.second, loc, clock.get()));
				pending_value->add_dep<mem_retire_value_t>(
					[loc](mem_retire_value_te){
					return e->data.addr == loc;
				});
			}
			++it;
		}
	}

	register_pending(pending_value);
	values->push_back(pending_value);
}

void vec1dmulti_t::process(pe_write_value_tvalue) {
	TIME_VIOLATION_CHECK
	uint16_t work = write_set.size() - active_reg_writes;
	if(work > rf_ports) work = rf_ports;

	pending_value_tpending_value;
	if(active_reg_writes < write_set.size()) {
		pending_value = new pending_value_t(this, 
			new pe_write_value_t(this, value->data), clock.get() + 1);
		pending_value->add_fini([&, work](){ active_reg_writes += work; });
	} else if(progress + lanes < vl) {
		pe_state = READ;
		active_reg_writes = 0; 
		progress += lanes;
		pending_value = new pending_value_t(this, 
			new pe_write_value_t(this, value->data), clock.get() + 1);
		pending_value->add_dep([&](){ return pe_state == WRITE; });
	} else {
		pending_value = new pending_value_t(this, 
			new pe_ready_value_t(this, retire_insn), clock.get() + 1);
		pending_value->add_fini([&](){ 
			vcu_t::set_core_stage("exec", false);
		});
	}

	auto it = std::next(write_set.begin(), active_reg_writes);
	auto end = std::next(write_set.begin(), active_reg_writes + work);
	while(it != end) {
		auto reg = *it;
		values->push_back(new vec_reg_write_value_t(
			this, {.reg=reg, .idx=0}, clock.get()));
		pending_value->add_dep<vec_reg_write_value_t>(
			[reg](vec_reg_write_value_te){
				return e->data.reg == reg;
		});
		++it;
	}
	register_pending(pending_value);
	values->push_back(pending_value);
}

void vec1dmulti_t::process(pe_ready_value_tvalue) {
	TIME_VIOLATION_CHECK
	idx = 0;
	pe_state = READ;
	active_window_size = 0;
	active_insn_offset = 0;
	active_reg_reads = 0;
	active_reg_writes = 0;
	progress = 0;
	write_set.clear();
	read_set.clear();
	empty = true;
	for(auto parent : parents.raw<vec_signal_handler_t *>()) {
		values->push_back(
			new vec_ready_value_t(parent.second, retire_insn, clock.get()));
		values->push_back(
			new vec_retire_value_t(parent.second, retire_insn, clock.get()));
	}
}