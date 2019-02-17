#include "vec1dmulti.h"

#include "mem_event.h"
#include "vector_event.h"
#include "pending_event.h"

vec1dmulti_t::vec1dmulti_t(std::string _name, io::json _config, 
	event_heap_t *_events) : vcu_t(_name, _config, _events) {
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
	start = false;
	active_window_size = 0;
	active_insn_offset = 0;
	active_reg_reads = 0;
	active_reg_writes = 0;
	progress = 0;
	write_set.clear();
	read_set.clear();
}

void vec1dmulti_t::process(vector_exec_event_t *event) {
	TIME_VIOLATION_CHECK;
	if(promote_pending(event, [&](){
		return !(active_window_size < window_size);
	}) != nullptr) return;
	empty = false;
	event->data->idx = idx;
	idx++;
	active_window_size++;
	// Add to table here
	for(auto it : event->data->ws.input.vregs) {
		uint8_t reg = strip_killed(it);
		bool found = write_set.find(reg) != write_set.end();
		if(!found) read_set.insert(reg);
		else if(check_killed(it) && found) write_set.erase(reg);
	}
	for(auto it : event->data->ws.output.vregs) {
		uint8_t reg = strip_killed(it);
		write_set.insert(reg);
	}
	if(active_window_size == window_size) {
		vcu_t::set_core_stage("exec", true);
		events->push_back(new vector_start_event_t(this, false, clock.get()));
		auto pending_event = new pending_event_t(this, 
		new pe_exec_event_t(this, event->data, clock.get()), clock.get() + 1);
		pending_event->add_dep([&]() { 
			return active_reg_reads >= read_set.size();
		});
		register_pending(pending_event);
		events->push_back(pending_event);
		return;
	}
	auto pending_event = new pending_event_t(this, 
		new pe_exec_event_t(this, event->data, clock.get()), clock.get() + 1);
	pending_event->add_dep<vector_start_event_t *>([](vector_start_event_t *e) {
		return true;
	});
	if(start){
		pending_event->add_dep([&]() { 
			return active_reg_reads >= read_set.size();
		});
	}
	start = true;
	register_pending(pending_event);
	events->push_back(pending_event);
}

void vec1dmulti_t::process(pe_exec_event_t *event) {
	TIME_VIOLATION_CHECK
	if(active_reg_reads < read_set.size()) { // Issue register reads
		uint16_t work = read_set.size();
		if(work > rf_ports) work = rf_ports;
		auto pending_event = new pending_event_t(this, 
			new pe_exec_event_t(this, event->data), clock.get() + 1);
		pending_event->add_fini([&, work](){
			active_reg_reads += work;
		});
		auto it = std::next(read_set.begin(), active_reg_reads);
		auto end = std::next(read_set.begin(), active_reg_reads + work);
		while(it != end) {
			auto reg = *it;
			events->push_back(new vector_reg_read_event_t(
				this, {.reg=reg, .idx=0}, clock.get()));
			pending_event->add_dep<vector_reg_read_event_t *>(
				[reg](vector_reg_read_event_t *e){
					return e->data.reg == reg;
			});
			++it;
		}
		register_pending(pending_event);
		events->push_back(pending_event);
	} else if(progress < vl) { // Compute
		// Promote to pending
		uint16_t insn_idx = event->data->idx;
		if(promote_pending(event, [&, insn_idx]() {
			return !(active_insn_offset == insn_idx && outstanding == 0);
		}) != nullptr) return;
		uint16_t work = vl - progress;
		if(work > lanes) work = lanes;
		auto pending_event = new pending_event_t(this, 
			new pe_exec_event_t(this, event->data), clock.get() + 1);
		pending_event->add_fini([&, insn_idx, work](){
			outstanding = 0;
			if(insn_idx == active_window_size) progress += work;
			active_insn_offset++;
			if(active_insn_offset + 1 == active_window_size) 
				active_insn_offset = 0;
		});

		// Input locations
		if(progress < event->data->ws.input.locs.size()) {
			auto it = std::next(event->data->ws.input.locs.begin(), progress);
			auto end = std::next(
				event->data->ws.input.locs.begin(), progress + work);
			while(it != end) {
				for(auto child : children.raw<ram_handler_t *>()) {
					auto loc = *it;
					events->push_back(
						new mem_read_event_t(child.second, loc, clock.get()));
					pending_event->add_dep<mem_retire_event_t *>(
						[loc](mem_retire_event_t *e){
						return e->data.addr == loc;
					});
					outstanding = 1;
				}
				++it;
			}
		}

		// Output locations
		if(progress < event->data->ws.output.locs.size()) {
			auto it = std::next(event->data->ws.output.locs.begin(), progress);
			auto end = std::next(
				event->data->ws.output.locs.begin(), progress + work);
			while(it != end) {
				for(auto child : children.raw<ram_handler_t *>()) {
					auto loc = *it;
					events->push_back(
						new mem_read_event_t(child.second, loc, clock.get()));
					pending_event->add_dep<mem_retire_event_t *>(
						[loc](mem_retire_event_t *e){
						return e->data.addr == loc;
					});
					outstanding = 1;
				}
				++it;
			}
		}
		register_pending(pending_event);
		events->push_back(pending_event);
	} else { // Issue register writes
		uint16_t work = write_set.size();
		pending_event_t *pending_event;
		if(work > rf_ports) {
			work = rf_ports;
			pending_event = new pending_event_t(this, 
				new pe_exec_event_t(this, event->data), clock.get() + 1);
			pending_event->add_fini([&, work](){
				active_reg_writes += work;
			});
		} else {
			pending_event = new pending_event_t(this, 
				new pe_ready_event_t(this, event->data), clock.get() + 1);
			pending_event->add_fini([&](){ 
				vcu_t::set_core_stage("exec", false);
			});
		}
		auto it = std::next(read_set.begin(), active_reg_writes);
		auto end = std::next(read_set.begin(), active_reg_writes + work);
		while(it != end) {
			auto reg = *it;
			events->push_back(new vector_reg_read_event_t(
				this, {.reg=reg, .idx=0}, clock.get()));
			pending_event->add_dep<vector_reg_write_event_t *>(
				[reg](vector_reg_write_event_t *e){
					return e->data.reg == reg;
			});
			++it;
		}
		register_pending(pending_event);
		events->push_back(pending_event);
	}
}

void vec1dmulti_t::process(pe_ready_event_t *event) {
	TIME_VIOLATION_CHECK
	idx = 0;
	start = false;
	active_window_size = 0;
	active_insn_offset = 0;
	active_reg_reads = 0;
	active_reg_writes = 0;
	progress = 0;
	write_set.clear();
	read_set.clear();
	empty = true;
	for(auto parent : parents.raw<vector_signal_handler_t *>()) {
		events->push_back(
			new vector_ready_event_t(parent.second, event->data, clock.get()));
		events->push_back(
			new vector_retire_event_t(parent.second, event->data, clock.get()));
	}
}