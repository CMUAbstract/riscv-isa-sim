#include "vec1d.h"

#include "mem_value.h"
#include "vector_value.h"
#include "pending_value.h"

vec1d_t::vec1d_t(std::string _name, io::json _config, value_heap_t *_values)
		: vcu_t(_name, _config, _values) {}

void vec1d_t::reset(reset_level_t level) {
	vcu_t::reset(level);
	idx = 0;
	active_lanes = 0;
}

void vec1d_t::process(vec_issue_value_tvalue) {
	TIME_VIOLATION_CHECK
	vcu_t::set_core_stage("exec", true);
	empty = false;
	values->push_back(new pe_exec_value_t(this, value->data, clock.get()));
}

void vec1d_t::process(pe_exec_value_tvalue) {
	TIME_VIOLATION_CHECK 

	if(promote_pending(value, [&](){
		return !(active_lanes < lanes);
	}) != nullptr) return;

	if(check_mul(value->data->opc)) {
		count["mul"].running.inc();
	} else {
		count["alu"].running.inc();
	}

	uint32_t remaining = vl - idx;
	if(remaining > lanes) remaining = lanes;
	active_lanes += remaining;

	pending_value_tpending_value;
	if(vl - idx > lanes) {
		pending_value = new pending_value_t(this, 
			new pe_exec_value_t(this, value->data), clock.get() + 1);
		pending_value->add_fini([&, remaining](){ 
			active_lanes -= remaining;
			idx += remaining;
		});
	} else {
		pending_value = new pending_value_t(this, 
			new pe_ready_value_t(this, value->data), clock.get() + 1);
		pending_value->add_fini([&, remaining](){ 
			active_lanes -= remaining; 
			vcu_t::set_core_stage("exec", false);
			empty = true;
		});
	}

	auto retire_value = new pending_value_t(this, nullptr, clock.get() + 1);
	retire_value->add_fini([&] { if(outstanding > 0) outstanding--; });
	uint32_t outstanding_inc = outstanding + 1;

	// Issue vector register file writes
	std::vector<pending_value_t> reg_pending_values;
	for(auto it : value->data->ws.output.vregs) {
		for(auto loc = idx; loc < idx + remaining; loc++) {
			if(value->data->ws.input.locs.size() == 0) {
				values->push_back(
					new vec_reg_write_value_t(
						this, {.reg=it, .idx=loc}, clock.get()));
			} else {
				reg_pending_values.push_back(new pending_value_t(
					this, new vec_reg_write_value_t(
						this, {.reg=it, .idx=loc}), clock.get()));
				register_pending(reg_pending_values.back());
				values->push_back(reg_pending_values.back());
			}
			pending_value->add_dep<vec_reg_write_value_t>(
				[it, loc](vec_reg_write_value_te){
					return e->data.reg == it && e->data.idx == loc;
			});
		}
	}

	if(idx < value->data->ws.input.locs.size()) {
		for(auto child : children.raw<ram_t *>()) {

			auto it = std::next(
				value->data->ws.input.locs.begin(), idx);
			auto end = std::next(
				value->data->ws.input.locs.begin(), idx + remaining);
			std::set<addr_t> locs;
			addr_t line_mask = ~(child.second->get_line_size() - 1);
			while(it != end) {
				locs.insert(*it & line_mask);
				++it;
			}

			for(auto loc : locs) {
				values->push_back(
					new mem_read_value_t(child.second, loc, clock.get()));
				pending_value->add_dep<mem_ready_value_t>(
					[loc](mem_ready_value_te){
					return e->data.addr == loc;
				});
				retire_value->add_dep<mem_retire_value_t>(
					[loc](mem_retire_value_te){
					return e->data.addr == loc;
				});
				outstanding = outstanding_inc;
				for(auto reg_pending : reg_pending_values) {
					reg_pending->add_dep<mem_retire_value_t>(
						[loc](mem_retire_value_te) {
						return e->data.addr == loc;
					});
				}
			}
		}
	}

	// Issue vector register file reads
	for(auto it : value->data->ws.input.vregs) {
		for(auto loc = idx; loc < idx + remaining; loc++) {
			values->push_back(
				new vec_reg_read_value_t(this, {.reg=it, .idx=loc}, clock.get()));
			pending_value->add_dep<vec_reg_read_value_t>(
				[it, loc](vec_reg_read_value_te){
					return e->data.reg == it && e->data.idx == loc;
			});
		}
	}

	if(idx < value->data->ws.output.locs.size()) {
		for(auto child : children.raw<ram_t *>()) {

			auto it = std::next(
				value->data->ws.output.locs.begin(), idx);
			auto end = std::next(
				value->data->ws.output.locs.begin(), idx + remaining);
			std::set<addr_t> locs;
			addr_t line_mask = ~(child.second->get_line_size() - 1);
			while(it != end) {
				locs.insert(*it & line_mask);
				++it;
			}

			for(auto loc : locs) {
				values->push_back(
					new mem_write_value_t(child.second, loc, clock.get()));
				pending_value->add_dep<mem_ready_value_t>(
					[loc](mem_ready_value_te){
					return e->data.addr == loc;
				});
				retire_value->add_dep<mem_retire_value_t>(
					[loc](mem_retire_value_te){
					return e->data.addr == loc;
				});
				outstanding = outstanding_inc;
			}
		}
	}

	register_pending(pending_value);
	register_pending(retire_value);
	values->push_back(pending_value);
	values->push_back(retire_value);
}

void vec1d_t::process(pe_ready_value_tvalue) {
	TIME_VIOLATION_CHECK
	idx = 0;
	for(auto parent : parents.raw<vec_signal_handler_t *>()) {
		values->push_back(
			new vec_ready_value_t(parent.second, value->data, clock.get()));
		// Check that all outstanding memory accesses have been retired
		auto vec_retire_value = new vec_retire_value_t(
			parent.second, value->data, clock.get());
		if(promote_pending(vec_retire_value, [&](){
			return outstanding != 0;
		}) != nullptr); 
		else values->push_back(vec_retire_value);
	}
}