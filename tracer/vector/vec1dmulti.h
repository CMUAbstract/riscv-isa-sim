#ifndef VEC1D_MULTI_H
#define VEC1D_MULTI_H

#include <set>
#include <vector>
#include <sstream>

#include "vcu.h"

class vec1dmulti_t: public vcu_t {
public:
	struct pe_read_value_t: public value_t<vec1dmulti_t, std::set<uint8_t> *> {
		using value_t<vec1dmulti_t, std::set<uint8_t> *>::value_t;
		std::string to_string() {
			std::ostringstream os;
			os << "pe_read_value_t (" << this->cycle << ", ";
			auto it = this->data->begin();
			auto end = std::prev(this->data->end(), 1);
			while(it != this->data->end()) {
				os << static_cast<uint16_t>(*it) << ", ";
				it++;
			}
			os << static_cast<uint16_t>(*it) << ")";
			return os.str();
		}
		std::string get_name() { return "pe_read_value_t"; }
		HANDLER;
	};
	struct pe_write_value_t: public value_t<vec1dmulti_t, std::set<uint8_t> *> {
		using value_t<vec1dmulti_t, std::set<uint8_t> *>::value_t;
		std::string to_string() {
			std::ostringstream os;
			os << "pe_write_value_t (" << this->cycle << ", ";
			auto it = this->data->begin();
			auto end = std::prev(this->data->end(), 1);
			while(it != this->data->end()) {
				os << static_cast<uint16_t>(*it) << ", ";
				it++;
			}
			os << static_cast<uint16_t>(*it) << ")";
			return os.str();
		}
		std::string get_name() { return "pe_write_value_t"; }
		HANDLER;
	};
public:
	vec1dmulti_t(std::string _name, io::json _config, value_heap_t *_values);
	io::json to_json() const;
	void reset(reset_level_t level);
	void process(vec_issue_value_tvalue);
	void process(pe_read_value_tvalue);
	void process(pe_exec_value_tvalue);
	void process(pe_write_value_tvalue);
	void process(pe_ready_value_tvalue);
protected:
	uint32_t window_size = 1;
	uint32_t rf_ports = 1;

	uint16_t idx = 0;
	uint16_t active_window_size = 0;
	uint16_t active_insn_offset = 0;
	uint16_t active_reg_reads = 0;
	uint16_t active_reg_writes = 0;
	uint16_t progress = 0;
	enum pe_state_t {READ, WRITE, EXEC};
	pe_state_t pe_state = READ; 

	std::set<uint8_t> write_set;
	std::set<uint8_t> read_set;
	hstd::shared_ptr<timed_insn_t> retire_insn;

};

#endif