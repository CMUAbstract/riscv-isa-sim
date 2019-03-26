#include "vec1d.h"

#include "mem_event.h"
#include "vector_event.h"
#include "pending_event.h"

vec1d_t::vec1d_t(std::string _name, io::json _config, event_heap_t *_events)
		: vcu_t(_name, _config, _events) {}

void vec1d_t::reset(reset_level_t level) {
	vcu_t::reset(level);
	// idx = 0;
	// active_lanes = 0;
}

void vec1d_t::process(vec_issue_event_t *event) {
	TIME_VIOLATION_CHECK
	vcu_t::set_core_stage("exec", true);
	empty = false;
	events->push_back(new pe_exec_event_t(this, event->data, clock.get()));
}

void vec1d_t::process(pe_exec_event_t *event) {
	TIME_VIOLATION_CHECK

	if(promote_pending(event, [&](){
		return !(active_lanes < lanes);
	}) != nullptr) return;

	count["alu"].running.inc();

	uint32_t remaining = vl - idx;
	if(remaining > lanes) remaining = lanes;
	active_lanes += remaining;

	pending_event_t *pending_event;
	if(vl - idx > lanes) {
		pending_event = new pending_event_t(this, 
			new pe_exec_event_t(this, event->data), clock.get() + 1);
		pending_event->add_fini([&, remaining](){ 
			active_lanes -= remaining;
			idx += remaining;
		});
	} else {
		pending_event = new pending_event_t(this, 
			new pe_ready_event_t(this, event->data), clock.get() + 1);
		pending_event->add_fini([&, remaining](){ 
			active_lanes -= remaining; 
			vcu_t::set_core_stage("exec", false);
			empty = true;
		});
	}

	auto retire_event = new pending_event_t(this, nullptr, clock.get() + 1);
	retire_event->add_fini([&] { if(outstanding > 0) outstanding--; });
	uint32_t outstanding_inc = outstanding + 1;

	if(idx < event->data->ws.input.locs.size()) {
		auto it = std::next(event->data->ws.input.locs.begin(), idx);
		auto end = std::next(event->data->ws.input.locs.begin(), idx + remaining);
		std::vector<addr_t> locs(it, end);
		for(auto child : children.raw<ram_t *>()) {
			addr_t last_bank = 0;
			addr_t last_addr = 0;
			for(auto loc : locs) {
				if(last_bank == child.second->get_bank(loc) &&
					loc < last_addr + child.second->get_line_size()) continue;
				events->push_back(
					new mem_read_event_t(child.second, loc, clock.get()));
				pending_event->add_dep<mem_ready_event_t *>(
					[loc](mem_ready_event_t *e){
					return e->data.addr == loc;
				});
				retire_event->add_dep<mem_retire_event_t *>(
					[loc](mem_retire_event_t *e){
					return e->data.addr == loc;
				});
				outstanding = outstanding_inc;
				last_addr = loc;
				last_bank = child.second->get_bank(loc);
			}
		}
	}

	// Issue vector register file reads
	for(auto it : event->data->ws.input.vregs) {
		for(auto loc = idx; loc < idx + remaining; loc++) {
			events->push_back(
				new vec_reg_read_event_t(this, {.reg=it, .idx=loc}, clock.get()));
			pending_event->add_dep<vec_reg_read_event_t *>(
				[it, loc](vec_reg_read_event_t *e){
					return e->data.reg == it && e->data.idx == loc;
			});
		}
	}

	if(idx < event->data->ws.output.locs.size()) {
		auto it = std::next(event->data->ws.output.locs.begin(), idx);
		auto end = std::next(event->data->ws.output.locs.begin(), idx + remaining);
		std::vector<addr_t> locs(it, end);
		for(auto child : children.raw<ram_t *>()) {
			addr_t last_bank = 0;
			addr_t last_addr = 0;
			for(auto loc : locs) {
				if(last_bank == child.second->get_bank(loc) &&
					loc < last_addr + child.second->get_line_size()) continue;
				events->push_back(
					new mem_write_event_t(child.second, loc, clock.get()));
				pending_event->add_dep<mem_ready_event_t *>(
					[loc](mem_ready_event_t *e){
					return e->data.addr == loc;
				});
				retire_event->add_dep<mem_retire_event_t *>(
					[loc](mem_retire_event_t *e){
					return e->data.addr == loc;
				});
				outstanding = outstanding_inc;
				last_addr = loc;
				last_bank = child.second->get_bank(loc);
			}
		}
	}

	// Issue vector register file writes
	uint32_t latency = 0;
	if(event->data->ws.input.vregs.size() > 0) latency = 1;
	for(auto it : event->data->ws.output.vregs) {
		for(auto loc = idx; loc < idx + remaining; loc++) {
			events->push_back(
				new vec_reg_write_event_t(
					this, {.reg=it, .idx=loc}, clock.get() + latency));
			pending_event->add_dep<vec_reg_write_event_t *>(
				[it, loc](vec_reg_write_event_t *e){
					return e->data.reg == it && e->data.idx == loc;
			});
		}
	}

	register_pending(pending_event);
	register_pending(retire_event);
	events->push_back(pending_event);
	events->push_back(retire_event);
}

void vec1d_t::process(pe_ready_event_t *event) {
	TIME_VIOLATION_CHECK
	idx = 0;
	for(auto parent : parents.raw<vec_signal_handler_t *>()) {
		events->push_back(
			new vec_ready_event_t(parent.second, event->data, clock.get()));
		// Check that all outstanding memory accesses have been retired
		auto vec_retire_event = new vec_retire_event_t(
			parent.second, event->data, clock.get());
		if(promote_pending(vec_retire_event, [&](){
			return outstanding != 0;
		}) != nullptr); 
		else events->push_back(vec_retire_event);
	}
}