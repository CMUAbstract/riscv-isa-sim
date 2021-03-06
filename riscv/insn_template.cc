// See LICENSE for license details.

#include "insn_template.h"

reg_t rv32_NAME(processor_t* p, insn_t insn, reg_t pc)
{
  int xlen = 32;
  reg_t npc = sext_xlen(pc + insn_length(OPCODE));
  working_set_t ws;
  #include <tracer/tracer_macro.h>
  if(p->get_tracer() != nullptr) {
    ws.pc = STATE.pc;
    ws.next_pc = npc;
    #include "insns/NAME.h"
    if(p->get_tracer()->interested(ws, OPCODE, insn)) {
      p->get_tracer()->trace(ws, OPCODE, insn);
    }
  } else {
    #include "insns/NAME.h"
  }
  return npc;
}

reg_t rv64_NAME(processor_t* p, insn_t insn, reg_t pc)
{
  int xlen = 64;
  reg_t npc = sext_xlen(pc + insn_length(OPCODE));
  working_set_t ws;
  #include <tracer/tracer_macro.h>
  if(p->get_tracer() != nullptr) {
    ws.pc = STATE.pc; 
    ws.next_pc = npc;
    #include "insns/NAME.h"
    if(p->get_tracer()->interested(ws, OPCODE, insn)) {
      p->get_tracer()->trace(ws, OPCODE, insn);
    }
  } else {
    #include "insns/NAME.h"
  }
  return npc;
}
