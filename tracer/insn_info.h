#ifndef INSN_INFO_H
#define INSN_INFO_H

#include <common/decode.h>

#include "working_set.h"

struct insn_info_t {
	insn_info_t(working_set_t _ws, insn_bits_t _opc, insn_t _insn)
		: ws(_ws), opc(_opc), insn(_insn) {}
	working_set_t ws;
	insn_bits_t opc;
	insn_t insn;
	uint32_t idx = 0;
};

#endif