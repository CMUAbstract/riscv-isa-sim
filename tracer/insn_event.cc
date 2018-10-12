#include "insn_event.h"

#include "working_set.h"

timed_insn_t::timed_insn_t(shared_ptr_t<working_set_t>_ws, 
	insn_bits_t _opc, insn_t _insn)
	: ws(_ws), opc(_opc), insn(_insn) {}

timed_insn_t::timed_insn_t(timed_insn_t *_insn)
	: ws(_insn->ws), opc(_insn->opc), 
	insn(_insn->insn) {}