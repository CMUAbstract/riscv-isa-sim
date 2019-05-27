#include "rtl_tracer.h"

#include <iostream>

#include <io/io.h>
#include <fesvr/elfloader.h>
#include <common/decode.h>

#include "log.h"

#define SIGN_EXTEND(v) (0xFFFFFFFF00000000 | v)
#define REMOVE_SIGN_EXTEND(v) (0x00000000FFFFFFFF & v)

#define WORD_SIZE 4
#define OFFSET_MASK (WORD_SIZE - 1)
#define WORD_MASK ~OFFSET_MASK
#define WORD(w) (w & WORD_MASK)
#define OFFSET(w) (w & OFFSET_MASK)

rtl_tracer_t::rtl_tracer_t(io::json _config, elfloader_t *_elf)
		: tracer_impl_t("rtl_tracer", _config, _elf), 
		registers("registers"), locations("locations") {
	for(uint32_t i = 0; i < NXPR; i++) registers.insert(i, 0);
}

io::json rtl_tracer_t::to_json() const {
	return io::json::merge_objects(registers, locations);
}

void rtl_tracer_t::trace(const working_set_t &ws, 
	const insn_bits_t opc, const insn_t &insn) {
	if(!loaded) {
		auto symbols = elf->get_symbols();
		auto it = symbols.find("_start");
		assert_msg(it != symbols.end(), "_start not found");
		start_addr = it->second.e32.st_value;

		it = symbols.find("_start_end");
		assert_msg(it != symbols.end(), "_start_end not found");
		end_addr = it->second.e32.st_value;
		
		loaded = true;
	}

	if(ws.pc == SIGN_EXTEND(start_addr)) {
		tracking = true;
	}

	if(tracking) {
		for(auto reg : ws.update.regs) {
			uint32_t r;
			reg_t v;
			std::tie(r, v) = reg;
			registers.update({r, REMOVE_SIGN_EXTEND(v)});
		}

		std::map<addr_t, reg_t> word_aligned; 
		for(auto loc : ws.update.locs) {
			addr_t a;
			uint8_t v;
			std::tie(a, v) = loc;
			uint32_t offset = OFFSET(a);
			uint32_t word = WORD(a);
			auto it = word_aligned.find(word);
			if(it == word_aligned.end()) {
				word_aligned.insert({word, (int32_t)v << (offset << 3)});
			} else {
				word_aligned[word] = it->second | ((int32_t)v << (offset << 3));
			}
		}
		for(auto loc : word_aligned) locations.update(loc);
	}

	if(ws.pc == SIGN_EXTEND(end_addr)) {
		// Dump here
		std::cout << io::json::merge_objects(registers, locations).dump() << std::endl;
		tracking = false;
		exit(0); // Yes this is a hack but who really cares...
	}
}
