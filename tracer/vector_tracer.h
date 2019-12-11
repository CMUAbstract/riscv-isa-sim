#ifndef VECTOR_TRACER_H
#define VECTOR_TRACER_H

#include <stat/stat.h>

#include "tracer.h"

#define MAX_VL 0x40
#define VREG_COUNT 0x10

class vector_tracer_t : public tracer_impl_t {
public:
	vector_tracer_t(io::json _config, elfloader_t *_elf);
	~vector_tracer_t();
	bool interested(
		const working_set_t &ws, const insn_bits_t opc, const insn_t &insn);
	void trace(
		const working_set_t &ws, const insn_bits_t opc, const insn_t &insn);
	void tabulate() {}
	io::json to_json() const {
		return io::json::merge_objects(repeats, thru_mem, vcount, 
			io::json::object{{"vinsns", vinsns}});
	}
private:
	uint16_t vl;
	std::array<std::array<uint32_t, MAX_VL>, VREG_COUNT> vregs;
	std::set<addr_t> updated;
	std::vector<std::set<addr_t>> loads;
	std::vector<uint32_t> load_idxs;
private:
	class vinsn_stat_t : public stat_t {
	public:
		vinsn_stat_t(std::string _name="", std::string _desc="")
			: stat_t(_name, _desc) {}
		io::json to_json() const {
			return io::json::object{
				{"opc", opc},
				{"pc", pc},
				{"type", type},
				{"mask", mask},
				{"src1", src1},
				{"src2", src2},
				{"dest", dest},
				{"vl", vl},
				{"repeat", repeat}
			};
		}
	public:
		addr_t pc;
		insn_bits_t opc;
		std::string type;
		std::vector<bool> mask;
		uint16_t repeat;
		uint16_t src1;
		uint16_t src2;
		uint16_t dest;
		uint16_t vl;
	};
	std::vector<vinsn_stat_t *> vinsns;
	counter_stat_t<uint32_t> repeats;
	counter_stat_t<uint32_t> thru_mem;
	counter_stat_t<uint32_t> vcount;
};

#endif