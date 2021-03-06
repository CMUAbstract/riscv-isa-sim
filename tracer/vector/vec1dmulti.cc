#include "vec1dmulti.h"

#include "mem_event.h"
#include "vector_event.h"
#include "pending_event.h"

vec1dmulti_t::vec1dmulti_t(std::string _name, io::json _config, 
	event_heap_t *_events) : vcu_t(_name, _config, _events), retire_insn(nullptr) {
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

void vec1dmulti_t::process(vec_issue_event_t *event) {
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
		events->push_back(new vec_start_event_t(this, false, clock.get()));
	}

	auto pending_event = new pending_event_t(this, 
		new pe_exec_event_t(this, event->data, clock.get()), clock.get() + 1);
	pending_event->add_dep<vec_start_event_t *>([](vec_start_event_t *e) {
		return true;
	});
	pending_event->add_dep([&](){ return pe_state == EXEC; });
	register_pending(pending_event);
	events->push_back(pending_event);

	if(active_window_size == 1) {
		auto pending_read_event = new pending_event_t(this, 
			new pe_read_event_t(this, &read_set, clock.get()), clock.get() + 1);
		pending_read_event->add_dep<vec_start_event_t *>(
			[](vec_start_event_t *e) { return true; });
		register_pending(pending_read_event);
		events->push_back(pending_read_event);

		auto pending_write_event = new pending_event_t(this, 
			new pe_write_event_t(this, &write_set, clock.get()), clock.get() + 1);
		pending_write_event->add_dep<vec_start_event_t *>(
			[](vec_start_event_t *e) { return true; });
		pending_write_event->add_dep([&](){ return pe_state == WRITE; });
		register_pending(pending_write_event);
		events->push_back(pending_write_event);
	}
	retire_insn = event->data;
}

void vec1dmulti_t::process(pe_read_event_t *event) {
	TIME_VIOLATION_CHECK
	uint16_t work = read_set.size() - active_reg_reads;
	if(work > rf_ports) work = rf_ports;
	pending_event_t *pending_event;

	if(active_reg_writes + work < read_set.size()) {
		 pending_event = new pending_event_t(this, 
			new pe_read_event_t(this, event->data), clock.get() + 1);
		pending_event->add_fini([&, work](){ active_reg_reads += work; });
	} else if(progress + lanes < vl) {
		pe_state = EXEC;
		active_reg_reads = 0;
		pending_event = new pending_event_t(this, 
			new pe_read_event_t(this, event->data), clock.get() + 1);
		pending_event->add_dep([&](){ return pe_state == READ; });
	} else {
		pe_state = EXEC;
		active_reg_reads = 0;
		pending_event = new pending_event_t(this, nullptr, clock.get() + 1);
	}

	auto it = std::next(read_set.begin(), active_reg_reads);
	auto end = std::next(read_set.begin(), active_reg_reads + work);
	while(it != end) {
		auto reg = *it;
		events->push_back(new vec_reg_read_event_t(
			this, {.reg=reg, .idx=0}, clock.get()));
		pending_event->add_dep<vec_reg_read_event_t *>(
			[reg](vec_reg_read_event_t *e){
				return e->data.reg == reg;
		});
		++it;
	}
	register_pending(pending_event);
	events->push_back(pending_event);
}

void vec1dmulti_t::process(pe_exec_event_t *event) {
	TIME_VIOLATION_CHECK
	// Promote to pending
	uint16_t insn_idx = event->data->idx;
	if(promote_pending(event, [&, insn_idx]() {
		return !(active_insn_offset == insn_idx);
	}) != nullptr) return;

	uint16_t work = vl - progress;
	if(work > lanes) work = lanes;

	pending_event_t *pending_event;
	if(progress + lanes < vl){
		pending_event = new pending_event_t(this, 
			new pe_exec_event_t(this, event->data), clock.get() + 1);
		pending_event->add_dep([&, cur_progess=progress]() { 
			return pe_state == EXEC && cur_progess < progress; 
		});
	} else {
		pending_event = new pending_event_t(this, nullptr, clock.get() + 1);
	}

	// THIS IS SORT OF WRONG
	active_insn_offset++;
	if(insn_idx == active_window_size - 1) {
		pe_state = WRITE;
		active_insn_offset = 0;
	}

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
			}
			++it;
		}
	}

	register_pending(pending_event);
	events->push_back(pending_event);
}

void vec1dmulti_t::process(pe_write_event_t *event) {
	TIME_VIOLATION_CHECK
	uint16_t work = write_set.size() - active_reg_writes;
	if(work > rf_ports) work = rf_ports;

	pending_event_t *pending_event;
	if(active_reg_writes < write_set.size()) {
		pending_event = new pending_event_t(this, 
			new pe_write_event_t(this, event->data), clock.get() + 1);
		pending_event->add_fini([&, work](){ active_reg_writes += work; });
	} else if(progress + lanes < vl) {
		pe_state = READ;
		active_reg_writes = 0; 
		progress += lanes;
		pending_event = new pending_event_t(this, 
			new pe_write_event_t(this, event->data), clock.get() + 1);
		pending_event->add_dep([&](){ return pe_state == WRITE; });
	} else {
		pending_event = new pending_event_t(this, 
			new pe_ready_event_t(this, retire_insn), clock.get() + 1);
		pending_event->add_fini([&](){ 
			vcu_t::set_core_stage("exec", false);
		});
	}

	auto it = std::next(write_set.begin(), active_reg_writes);
	auto end = std::next(write_set.begin(), active_reg_writes + work);
	while(it != end) {
		auto reg = *it;
		events->push_back(new vec_reg_write_event_t(
			this, {.reg=reg, .idx=0}, clock.get()));
		pending_event->add_dep<vec_reg_write_event_t *>(
			[reg](vec_reg_write_event_t *e){
				return e->data.reg == reg;
		});
		++it;
	}
	register_pending(pending_event);
	events->push_back(pending_event);
}

void vec1dmulti_t::process(pe_ready_event_t *event) {
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
		events->push_back(
			new vec_ready_event_t(parent.second, retire_insn, clock.get()));
		events->push_back(
			new vec_retire_event_t(parent.second, retire_insn, clock.get()));
	}
}