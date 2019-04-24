#include "cache.h"

#include <bitset>
#include <string>

#include "log.h"
#include "event.h"
#include "mem_event.h"
#include "repl_policy.h"

#define CACHE_LOG 0

cache_t::cache_t(std::string _name, io::json _config, event_heap_t *_events)
	: ram_t(_name, _config, _events), writebacks("write_back") {
	JSON_CHECK(bool, config["write_thru"], write_thru, false);
	JSON_CHECK(int, config["lines"], lines, 64);
	JSON_CHECK(int, config["sets"], sets, 8);
	JSON_CHECK(int, config["invalid_latency"], invalid_latency, 1);

	std::string which_repl_policy;
	JSON_CHECK(string, config["repl_policy"], which_repl_policy);
	if(which_repl_policy.size() == 0) {
		repl_policy = new lru_repl_policy_t(lines); 
	} else {
		repl_policy = repl_policy_type_map.at(which_repl_policy)(lines); 
	}

	// Statistics to track
	track_energy("read_hit");
	track_energy("write_hit");
	track_energy("read_miss");
	track_energy("write_miss");

	writebacks.reset();

	offset_mask = line_size - 1; 
	uint32_t idx = line_size;
	while (idx >>= 1) ++set_offset;
	set_size = lines / sets;
	set_mask = (sets - 1) << set_offset;
	bank_mask = (bank_count - 1) << set_offset;
	tag_mask = ~(offset_mask | set_mask);
	data.resize(lines, 0);
	dirty.resize(lines, false);

#if CACHE_LOG
	std::cerr << name << std::endl;
	std::cerr << "offset_mask: " << std::bitset<32>(offset_mask) << std::endl;
	std::cerr << "set_mask: " << std::bitset<32>(set_mask) << std::endl;
	std::cerr << "set_offset: " << set_offset << std::endl;
	std::cerr << "tag_mask: " << std::bitset<32>(tag_mask) << std::endl;
	std::cerr << "line_size: " << line_size << std::endl;
	std::cerr << "lines: " << lines << std::endl;
#endif
}

void cache_t::reset(reset_level_t level) {
	ram_t::reset(level);
	repl_policy->reset();
	if(level == SOFT) return;
	std::fill(dirty.begin(), dirty.end(), false);
	std::fill(data.begin(), data.end(), false);
}

io::json cache_t::to_json() const {
	return io::json::merge_objects(ram_t::to_json(), writebacks);
}

void cache_t::process(mem_read_event_t *event) {
	TIME_VIOLATION_CHECK

	if(outstanding.count(get_line(event->data.addr)) != 0) {
		subline_outstanding.insert(event->data.addr);
		for(auto parent : parents.raw<ram_signal_handler_t *>()) {
			events->push_back(
				new mem_ready_event_t(
					parent.second, event->data, clock.get()));
		}
		return;
	}

	auto bank = get_bank(event->data.addr);
	if(promote_pending(event, [&, bank](){
		return !(banks[bank].readers < read_ports_per_bank &&
			banks[bank].total() < ports_per_bank);
	}) != nullptr) {
		bank_conflicts.inc();
		banks[bank].readerq++;
		if(banks[bank].readerq <= load_buf_size) {
			for(auto parent : parents.raw<ram_signal_handler_t *>()) {
				events->push_back(
					new mem_ready_event_t(
						parent.second, event->data, clock.get()));
			}
		}
		return;
	}

#if CACHE_LOG
	uint32_t set = get_set(event->data.addr);
	uint32_t tag = get_tag(event->data.addr);
	std::cerr << "READ: " << name << "(set: 0x" << std::hex << set;
	std::cerr << ", tag: 0x" << tag << ", addr:" << event->data.addr;
	std::cerr << ", clock: " << event->cycle << ")" << std::endl;
#endif

	// Increment readers
	banks[bank].readers++;
	if(banks[bank].readerq > 0) banks[bank].readerq--;
	auto pending_event = new pending_event_t(
		this, nullptr, clock.get() + read_latency);
	pending_event->add_fini([&, bank](){ banks[bank].readers--; });
	register_pending(pending_event);
	events->push_back(pending_event);

	if(!access(event)) { // Read Miss
		count["read_miss"].running.inc();
		outstanding.insert(get_line(event->data.addr));
		for(auto child : children.raw<ram_t *>()) {
			events->push_back(
				new mem_read_event_t(
					child.second, event->data, clock.get() + 1));
			auto pending_event = new pending_event_t (
				this, new mem_insert_event_t(this, event->data), clock.get() + 1);
			pending_event->add_dep<mem_retire_event_t *>(
				[addr=event->data.addr](mem_retire_event_t *e) {
				return e->data.addr == addr;
			});
			register_pending(pending_event);
			events->push_back(pending_event);
		}
		for(auto parent : parents.raw<ram_signal_handler_t *>()) {
			events->push_back(
				new mem_ready_event_t(
					parent.second, event->data, clock.get()));
		}
		return;
	}

	count["read_hit"].running.inc();
	for(auto parent : parents.raw<ram_signal_handler_t *>()) { // Blocking
		events->push_back(
			new mem_ready_event_t(
				parent.second, event->data, clock.get()));
		events->push_back(
			new mem_retire_event_t(
				parent.second, event->data, clock.get() + read_latency));
	}
}

void cache_t::process(mem_write_event_t *event) {
	TIME_VIOLATION_CHECK
	
	if(outstanding.count(get_line(event->data.addr)) != 0) {
		subline_outstanding.insert(event->data.addr);
		for(auto parent : parents.raw<ram_signal_handler_t *>()) {
			events->push_back(
				new mem_ready_event_t(
					parent.second, event->data, clock.get()));
		}
		return;
	}

	auto bank = get_bank(event->data.addr);
	if(promote_pending(event, [&, bank](){
		return !(banks[bank].writers < write_ports_per_bank &&
			banks[bank].total() < ports_per_bank);
	}) != nullptr) {
		bank_conflicts.inc();
		banks[bank].writerq++;
		if(banks[bank].writerq <= store_buf_size) {
			for(auto parent : parents.raw<ram_signal_handler_t *>()) {
				events->push_back(
					new mem_ready_event_t(
						parent.second, event->data, clock.get()));
			}
		}
		return;
	}

#if CACHE_LOG
	uint32_t set = get_set(event->data.addr);
	uint32_t tag = get_tag(event->data.addr);
	std::cerr << "WRITE: " << name << "(set: 0x" << std::hex << set;
	std::cerr << ", tag: 0x" << tag << ", addr:" << event->data.addr;
	std::cerr << ", clock: " << event->cycle << ")" << std::endl;
#endif
	// Increment writers
	banks[bank].writers++;
	if(banks[bank].writerq > 0) banks[bank].writerq--;
	auto pending_event = new pending_event_t(
		this, nullptr, clock.get() + read_latency);
	pending_event->add_fini([&, bank](){ banks[bank].writers--; });
	register_pending(pending_event);
	events->push_back(pending_event);

	if(!access(event)) { // Write Miss
		count["write_miss"].running.inc();
		outstanding.insert(get_line(event->data.addr));
		for(auto child : children.raw<ram_t *>()) {
			events->push_back(
				new mem_write_event_t(
					child.second, event->data, clock.get() + 1));
			auto pending_event = new pending_event_t (
				this, new mem_insert_event_t(this, event->data), clock.get() + 1);
			pending_event->add_dep<mem_retire_event_t *>(
				[addr=event->data.addr](mem_retire_event_t *e) {
				return e->data.addr == addr;
			});
			register_pending(pending_event);
			events->push_back(pending_event);
		}
		for(auto parent : parents.raw<ram_signal_handler_t *>()) {
			events->push_back(
				new mem_ready_event_t(
					parent.second, event->data, clock.get()));
		}
		return;
	}

	count["write_hit"].running.inc();

	if(write_thru) {
		std::vector<pending_event_t *> retire_pending_events;
		std::vector<pending_event_t *> ready_pending_events;

		for(auto parent : parents.raw<ram_signal_handler_t *>()) {
			retire_pending_events.push_back(
				new pending_event_t(this, 
					new mem_retire_event_t(
						parent.second, event->data), clock.get() + 1));
			ready_pending_events.push_back(
				new pending_event_t(this, 
					new mem_ready_event_t(
						parent.second, event->data), clock.get()));
		}

		for(auto child : children.raw<ram_t *>()) {
			events->push_back(
				new mem_write_event_t(
					child.second, event->data, clock.get()));
			
			for(auto pe : retire_pending_events) {
				pe->add_dep<mem_retire_event_t *>(
					[addr=event->data.addr](mem_retire_event_t *e){
					return e->data.addr == addr;
				});
			}

			for(auto pe : ready_pending_events) {
				pe->add_dep<mem_ready_event_t *>(
					[addr=event->data.addr](mem_ready_event_t *e){
					return e->data.addr == addr;
				});
			}
		}

		for(auto pe : retire_pending_events) {
			register_pending(pe);
			events->push_back(pe);
		}

		for(auto pe : ready_pending_events) {
			register_pending(pe);
			events->push_back(pe);
		}
	} else {
		set_dirty(event); // Mark line as dirty
		for(auto parent : parents.raw<ram_signal_handler_t *>()) {
			events->push_back(
				new mem_ready_event_t(
					parent.second, event->data, clock.get()));
			events->push_back(
				new mem_retire_event_t(
					parent.second, event->data, clock.get() + write_latency));
		}
	}
}

void cache_t::process(mem_insert_event_t *event) {
	TIME_VIOLATION_CHECK
	check_pending(event);
	auto bank = get_bank(event->data.addr);
	if(promote_pending(event, [&, bank](){
		return !(banks[bank].writers < write_ports_per_bank &&
			banks[bank].total() < ports_per_bank);
	}) != nullptr) {
		banks[bank].writerq++;
		if(banks[bank].writerq <= store_buf_size) {
			for(auto parent : parents.raw<ram_signal_handler_t *>()) {
				events->push_back(
					new mem_ready_event_t(
						parent.second, event->data, clock.get() + 1));
			}
		}
		return;
	}

	// Increment writers
	banks[bank].writers++;
	if(banks[bank].writerq > 0) banks[bank].writerq--;
	if(!event->data.reader && banks[bank].writerq > 0) banks[bank].writerq--;
	if(event->data.reader && banks[bank].readerq > 0) banks[bank].readerq--;
	auto pending_event = new pending_event_t(
		this, nullptr, clock.get() + read_latency);
	pending_event->add_fini([&, bank](){ banks[bank].writers--; });
	register_pending(pending_event);
	events->push_back(pending_event);

	outstanding.erase(get_line(event->data.addr));

	uint32_t set = get_set(event->data.addr);
	uint32_t tag = get_tag(event->data.addr);
	std::vector<repl_cand_t> cands; // Create a set of candidates
	uint32_t id = set * set_size;
	for(id; id < (set + 1) * set_size; id++) cands.push_back(id);
	id = repl_policy->rank(event, &cands); // find which to replace
	data[id] = event->data.addr & tag_mask; // record new element in cache
	repl_policy->replaced(id); // tell repl policy element replaced

	if(dirty[id] && !write_thru) { // Determine if write back needed
		writebacks.inc();
		for(auto child : children.raw<ram_t *>()) {
			events->push_back(
				new mem_write_event_t(
					child.second, event->data, clock.get() + write_latency));
		}
	}

	if(event->data.reader && !write_thru) {
		dirty[id] = false;
	} else if(!event->data.reader && write_thru) {
		// Issue a write because insert was initiated by a write and cache is write
		// through
		for(auto child : children.raw<ram_t *>()) {
			events->push_back(
				new mem_write_event_t(
					child.second, event->data, clock.get() + write_latency));
		}
	}

	std::vector<addr_t> subline_remove;
	for(auto parent : parents.raw<ram_signal_handler_t *>()) {
		events->push_back(
			new mem_retire_event_t(
				parent.second, event->data, clock.get() + invalid_latency));
		for(auto sub : subline_outstanding) {
			if(sub == event->data.addr) continue;
			mem_event_info_t mem_event_info = {.addr=sub, .reader=false};
			events->push_back(new mem_retire_event_t(
				parent.second, mem_event_info, clock.get() + invalid_latency));
			subline_remove.push_back(sub);
		}
	}
	for(auto sub : subline_remove) subline_outstanding.erase(sub);
}

bool cache_t::access(mem_event_t *event) {
	uint32_t set = get_set(event->data.addr);
	uint32_t tag = get_tag(event->data.addr);
	for(uint32_t id = set * set_size; id < (set + 1) * set_size; id++) {
		if((data[id] & tag_mask) == tag) {
			repl_policy->update(id, event);
#if CACHE_LOG
			std::cerr << "	HIT: ";
			std::cerr << name;
			std::cerr << " => access (set: 0x" << std::hex << set;
			std::cerr << " tag: 0x" << tag << ")" << std::endl;
#endif
			return true;
		}
	}
#if CACHE_LOG
	std::cerr << "	MISS: ";
	std::cerr << name;
	std::cerr << " => access (set: 0x" << std::hex << set;
	std::cerr << " tag: 0x" << tag << ")" << std::endl;
#endif
	return false;
}

void cache_t::set_dirty(mem_event_t *event) {
	uint32_t set = get_set(event->data.addr);
	uint32_t tag = get_tag(event->data.addr);
	for(uint32_t id = set * set_size; id < (set + 1) * set_size; id++) 
		if((data[id] & tag_mask) == tag) dirty[id] = true;
}

addr_t cache_t::get_line(addr_t addr) {
	return addr & (~offset_mask);
}

addr_t cache_t::get_set(addr_t addr) {
	return (addr & set_mask) >> set_offset;
}

addr_t cache_t::get_tag(addr_t addr) {
	return addr & tag_mask;
}

addr_t cache_t::get_bank(addr_t addr) {
	return (addr & bank_mask) >> set_offset;
}