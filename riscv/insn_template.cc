// See LICENSE for license details.

#include "insn_template.h"

reg_t rv32_NAME(processor_t* p, insn_t insn, reg_t pc)
{
  int xlen = 32;
  reg_t npc = sext_xlen(pc + insn_length(OPCODE));
  working_set_t ws;
  #include "tracer_macro.h"
  #include "insns/NAME.h"
  if(p->get_tracer()->interested(p, OPCODE, insn, ws)) {
    p->get_tracer()->trace(p, OPCODE, insn, ws);
  }
  return npc;
}

reg_t rv64_NAME(processor_t* p, insn_t insn, reg_t pc)
{
  int xlen = 64;
  reg_t npc = sext_xlen(pc + insn_length(OPCODE));
  working_set_t ws;
  #include "tracer_macro.h"
  #include "insns/NAME.h"
  if(p->get_tracer()->interested(p, OPCODE, insn, ws)) {
    p->get_tracer()->trace(p, OPCODE, insn, ws);
  }
  return npc;
}
