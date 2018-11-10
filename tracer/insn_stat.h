#ifndef RISCV_INSN_STAT
#define RISCV_INSN_STAT

DEFINE_INSN_STAT(MATCH_ADD, ARITH0)
DEFINE_INSN_STAT(MATCH_ADDI, ARITH0)
DEFINE_INSN_STAT(MATCH_ADDIW, ARITH0)
DEFINE_INSN_STAT(MATCH_ADDW, ARITH0)
DEFINE_INSN_STAT(MATCH_AMOADD_D, ARITH0)
DEFINE_INSN_STAT(MATCH_AMOADD_W, ARITH0)
DEFINE_INSN_STAT(MATCH_AMOAND_D, ARITH0)
DEFINE_INSN_STAT(MATCH_AMOAND_W, ARITH0)
DEFINE_INSN_STAT(MATCH_AMOMAX_D, ARITH0)
DEFINE_INSN_STAT(MATCH_AMOMAXU_D, ARITH0)
DEFINE_INSN_STAT(MATCH_AMOMAXU_W, ARITH0)
DEFINE_INSN_STAT(MATCH_AMOMAX_W, ARITH0)
DEFINE_INSN_STAT(MATCH_AMOMIN_D, ARITH0)
DEFINE_INSN_STAT(MATCH_AMOMINU_D, ARITH0)
DEFINE_INSN_STAT(MATCH_AMOMINU_W, ARITH0)
DEFINE_INSN_STAT(MATCH_AMOMIN_W, ARITH0)
DEFINE_INSN_STAT(MATCH_AMOOR_D, ARITH0)
DEFINE_INSN_STAT(MATCH_AMOOR_W, ARITH0)
DEFINE_INSN_STAT(MATCH_AMOSWAP_D, ARITH0)
DEFINE_INSN_STAT(MATCH_AMOSWAP_W, ARITH0)
DEFINE_INSN_STAT(MATCH_AMOXOR_D, ARITH0)
DEFINE_INSN_STAT(MATCH_AMOXOR_W, ARITH0)
DEFINE_INSN_STAT(MATCH_AND, ARITH0)
DEFINE_INSN_STAT(MATCH_ANDI, ARITH0)
DEFINE_INSN_STAT(MATCH_AUIPC, CONTROL)
DEFINE_INSN_STAT(MATCH_BEQ, CONTROL)
DEFINE_INSN_STAT(MATCH_BGE, CONTROL)
DEFINE_INSN_STAT(MATCH_BGEU, CONTROL)
DEFINE_INSN_STAT(MATCH_BLT, CONTROL)
DEFINE_INSN_STAT(MATCH_BLTU, CONTROL)
DEFINE_INSN_STAT(MATCH_BNE, CONTROL)
DEFINE_INSN_STAT(MATCH_C_ADD, ARITH0)
DEFINE_INSN_STAT(MATCH_C_ADDI4SPN, ARITH0)
DEFINE_INSN_STAT(MATCH_C_ADDI, ARITH0)
DEFINE_INSN_STAT(MATCH_C_ADDW, ARITH0)
DEFINE_INSN_STAT(MATCH_C_AND, ARITH0)
DEFINE_INSN_STAT(MATCH_C_ANDI, ARITH0)
DEFINE_INSN_STAT(MATCH_C_BEQZ, CONTROL)
DEFINE_INSN_STAT(MATCH_C_BNEZ, CONTROL)
DEFINE_INSN_STAT(MATCH_C_EBREAK, CONTROL)
DEFINE_INSN_STAT(MATCH_C_FLD, LOAD)
DEFINE_INSN_STAT(MATCH_C_FLDSP, LOAD)
DEFINE_INSN_STAT(MATCH_C_FLW, LOAD)
DEFINE_INSN_STAT(MATCH_C_FLWSP, LOAD)
DEFINE_INSN_STAT(MATCH_C_FSD, LOAD)
DEFINE_INSN_STAT(MATCH_C_FSDSP, LOAD)
DEFINE_INSN_STAT(MATCH_C_FSW, LOAD)
DEFINE_INSN_STAT(MATCH_C_FSWSP, LOAD)
DEFINE_INSN_STAT(MATCH_C_JAL, CONTROL)
DEFINE_INSN_STAT(MATCH_C_JALR, CONTROL)
DEFINE_INSN_STAT(MATCH_C_J, CONTROL)
DEFINE_INSN_STAT(MATCH_C_JR, CONTROL)
DEFINE_INSN_STAT(MATCH_C_LI, CONTROL)
DEFINE_INSN_STAT(MATCH_C_LUI, CONTROL)
DEFINE_INSN_STAT(MATCH_C_LW, LOAD)
DEFINE_INSN_STAT(MATCH_C_LWSP, LOAD)
DEFINE_INSN_STAT(MATCH_C_MV, CONTROL)
DEFINE_INSN_STAT(MATCH_C_OR, ARITH0)
DEFINE_INSN_STAT(MATCH_C_SLLI, ARITH0)
DEFINE_INSN_STAT(MATCH_C_SRAI, ARITH0)
DEFINE_INSN_STAT(MATCH_C_SRLI, ARITH0)
DEFINE_INSN_STAT(MATCH_C_SUB, ARITH0)
DEFINE_INSN_STAT(MATCH_C_SUBW, ARITH0)
DEFINE_INSN_STAT(MATCH_C_XOR, ARITH0)
DEFINE_INSN_STAT(MATCH_CSRRC, ARITH0)
DEFINE_INSN_STAT(MATCH_CSRRCI, ARITH0)
DEFINE_INSN_STAT(MATCH_CSRRS, ARITH0)
DEFINE_INSN_STAT(MATCH_CSRRSI, ARITH0)
DEFINE_INSN_STAT(MATCH_CSRRW, ARITH0)
DEFINE_INSN_STAT(MATCH_CSRRWI, ARITH0)
DEFINE_INSN_STAT(MATCH_C_SW, STORE)
DEFINE_INSN_STAT(MATCH_C_SWSP, STORE)
DEFINE_INSN_STAT(MATCH_DIV, ARITH1)
DEFINE_INSN_STAT(MATCH_DIVU, ARITH1)
DEFINE_INSN_STAT(MATCH_DIVUW, ARITH1)
DEFINE_INSN_STAT(MATCH_DIVW, ARITH1)
DEFINE_INSN_STAT(MATCH_DRET, CONTROL)
DEFINE_INSN_STAT(MATCH_EBREAK, CONTROL)
DEFINE_INSN_STAT(MATCH_ECALL, CONTROL)
DEFINE_INSN_STAT(MATCH_FADD_D, ARITH2)
DEFINE_INSN_STAT(MATCH_FADD_Q, ARITH2)
DEFINE_INSN_STAT(MATCH_FADD_S, ARITH2)
DEFINE_INSN_STAT(MATCH_FCLASS_D, ARITH2)
DEFINE_INSN_STAT(MATCH_FCLASS_Q, ARITH2)
DEFINE_INSN_STAT(MATCH_FCLASS_S, ARITH2)
DEFINE_INSN_STAT(MATCH_FCVT_D_L, ARITH2)
DEFINE_INSN_STAT(MATCH_FCVT_D_LU, ARITH2)
DEFINE_INSN_STAT(MATCH_FCVT_D_Q, ARITH2)
DEFINE_INSN_STAT(MATCH_FCVT_D_S, ARITH2)
DEFINE_INSN_STAT(MATCH_FCVT_D_W, ARITH2)
DEFINE_INSN_STAT(MATCH_FCVT_D_WU, ARITH2)
DEFINE_INSN_STAT(MATCH_FCVT_L_D, ARITH2)
DEFINE_INSN_STAT(MATCH_FCVT_L_Q, ARITH2)
DEFINE_INSN_STAT(MATCH_FCVT_L_S, ARITH2)
DEFINE_INSN_STAT(MATCH_FCVT_LU_D, ARITH2)
DEFINE_INSN_STAT(MATCH_FCVT_LU_Q, ARITH2)
DEFINE_INSN_STAT(MATCH_FCVT_LU_S, ARITH2)
DEFINE_INSN_STAT(MATCH_FCVT_Q_D, ARITH2)
DEFINE_INSN_STAT(MATCH_FCVT_Q_L, ARITH2)
DEFINE_INSN_STAT(MATCH_FCVT_Q_LU, ARITH2)
DEFINE_INSN_STAT(MATCH_FCVT_Q_S, ARITH2)
DEFINE_INSN_STAT(MATCH_FCVT_Q_W, ARITH2)
DEFINE_INSN_STAT(MATCH_FCVT_Q_WU, ARITH2)
DEFINE_INSN_STAT(MATCH_FCVT_S_D, ARITH2)
DEFINE_INSN_STAT(MATCH_FCVT_S_L, ARITH2)
DEFINE_INSN_STAT(MATCH_FCVT_S_LU, ARITH2)
DEFINE_INSN_STAT(MATCH_FCVT_S_Q, ARITH2)
DEFINE_INSN_STAT(MATCH_FCVT_S_W, ARITH2)
DEFINE_INSN_STAT(MATCH_FCVT_S_WU, ARITH2)
DEFINE_INSN_STAT(MATCH_FCVT_W_D, ARITH2)
DEFINE_INSN_STAT(MATCH_FCVT_W_Q, ARITH2)
DEFINE_INSN_STAT(MATCH_FCVT_W_S, ARITH2)
DEFINE_INSN_STAT(MATCH_FCVT_WU_D, ARITH2)
DEFINE_INSN_STAT(MATCH_FCVT_WU_Q, ARITH2)
DEFINE_INSN_STAT(MATCH_FCVT_WU_S, ARITH2)
DEFINE_INSN_STAT(MATCH_FDIV_D, ARITH3)
DEFINE_INSN_STAT(MATCH_FDIV_Q, ARITH3)
DEFINE_INSN_STAT(MATCH_FDIV_S, ARITH3)
DEFINE_INSN_STAT(MATCH_FENCE, CONTROL)
DEFINE_INSN_STAT(MATCH_FENCE_I, CONTROL)
DEFINE_INSN_STAT(MATCH_FEQ_D, ARITH2)
DEFINE_INSN_STAT(MATCH_FEQ_Q, ARITH2)
DEFINE_INSN_STAT(MATCH_FEQ_S, ARITH2)
DEFINE_INSN_STAT(MATCH_FLD, LOAD)
DEFINE_INSN_STAT(MATCH_FLE_D, CONTROL)
DEFINE_INSN_STAT(MATCH_FLE_Q, CONTROL)
DEFINE_INSN_STAT(MATCH_FLE_S, CONTROL)
DEFINE_INSN_STAT(MATCH_FLQ, CONTROL)
DEFINE_INSN_STAT(MATCH_FLT_D, CONTROL)
DEFINE_INSN_STAT(MATCH_FLT_Q, CONTROL)
DEFINE_INSN_STAT(MATCH_FLT_S, CONTROL)
DEFINE_INSN_STAT(MATCH_FLW, LOAD)
DEFINE_INSN_STAT(MATCH_FMADD_D, ARITH3)
DEFINE_INSN_STAT(MATCH_FMADD_Q, ARITH3)
DEFINE_INSN_STAT(MATCH_FMADD_S, ARITH3)
DEFINE_INSN_STAT(MATCH_FMAX_D, ARITH2)
DEFINE_INSN_STAT(MATCH_FMAX_Q, ARITH2)
DEFINE_INSN_STAT(MATCH_FMAX_S, ARITH2)
DEFINE_INSN_STAT(MATCH_FMIN_D, ARITH2)
DEFINE_INSN_STAT(MATCH_FMIN_Q, ARITH2)
DEFINE_INSN_STAT(MATCH_FMIN_S, ARITH2)
DEFINE_INSN_STAT(MATCH_FMSUB_D, ARITH3)
DEFINE_INSN_STAT(MATCH_FMSUB_Q, ARITH3)
DEFINE_INSN_STAT(MATCH_FMSUB_S, ARITH3)
DEFINE_INSN_STAT(MATCH_FMUL_D, ARITH3)
DEFINE_INSN_STAT(MATCH_FMUL_Q, ARITH3)
DEFINE_INSN_STAT(MATCH_FMUL_S, ARITH3)
DEFINE_INSN_STAT(MATCH_FMV_D_X, CONTROL)
DEFINE_INSN_STAT(MATCH_FMV_W_X, CONTROL)
DEFINE_INSN_STAT(MATCH_FMV_X_D, CONTROL)
DEFINE_INSN_STAT(MATCH_FMV_X_W, CONTROL)
DEFINE_INSN_STAT(MATCH_FNMADD_D, ARITH3)
DEFINE_INSN_STAT(MATCH_FNMADD_Q, ARITH3)
DEFINE_INSN_STAT(MATCH_FNMADD_S, ARITH3)
DEFINE_INSN_STAT(MATCH_FNMSUB_D, ARITH3)
DEFINE_INSN_STAT(MATCH_FNMSUB_Q, ARITH3)
DEFINE_INSN_STAT(MATCH_FNMSUB_S, ARITH3)
DEFINE_INSN_STAT(MATCH_FSD, STORE)
DEFINE_INSN_STAT(MATCH_FSGNJ_D, ARITH2)
DEFINE_INSN_STAT(MATCH_FSGNJ_Q, ARITH2)
DEFINE_INSN_STAT(MATCH_FSGNJN_D, ARITH2)
DEFINE_INSN_STAT(MATCH_FSGNJN_Q, ARITH2)
DEFINE_INSN_STAT(MATCH_FSGNJN_S, ARITH2)
DEFINE_INSN_STAT(MATCH_FSGNJ_S, ARITH2)
DEFINE_INSN_STAT(MATCH_FSGNJX_D, ARITH2)
DEFINE_INSN_STAT(MATCH_FSGNJX_Q, ARITH2)
DEFINE_INSN_STAT(MATCH_FSGNJX_S, ARITH2)
DEFINE_INSN_STAT(MATCH_FSQ, ARITH3)
DEFINE_INSN_STAT(MATCH_FSQRT_D, ARITH3)
DEFINE_INSN_STAT(MATCH_FSQRT_Q, ARITH3)
DEFINE_INSN_STAT(MATCH_FSQRT_S, ARITH3)
DEFINE_INSN_STAT(MATCH_FSUB_D, ARITH2)
DEFINE_INSN_STAT(MATCH_FSUB_Q, ARITH2)
DEFINE_INSN_STAT(MATCH_FSUB_S, ARITH2)
DEFINE_INSN_STAT(MATCH_FSW, STORE)
DEFINE_INSN_STAT(MATCH_JAL, CONTROL)
DEFINE_INSN_STAT(MATCH_JALR, CONTROL)
DEFINE_INSN_STAT(MATCH_LB, LOAD)
DEFINE_INSN_STAT(MATCH_LBU, LOAD)
DEFINE_INSN_STAT(MATCH_LD, LOAD)
DEFINE_INSN_STAT(MATCH_LH, LOAD)
DEFINE_INSN_STAT(MATCH_LHU, LOAD)
DEFINE_INSN_STAT(MATCH_LR_D, LOAD)
DEFINE_INSN_STAT(MATCH_LR_W, LOAD)
DEFINE_INSN_STAT(MATCH_LUI, CONTROL)
DEFINE_INSN_STAT(MATCH_LW, LOAD)
DEFINE_INSN_STAT(MATCH_LWU, LOAD)
DEFINE_INSN_STAT(MATCH_MRET, CONTROL)
DEFINE_INSN_STAT(MATCH_MUL, ARITH1)
DEFINE_INSN_STAT(MATCH_MULH, ARITH1)
DEFINE_INSN_STAT(MATCH_MULHSU, ARITH1)
DEFINE_INSN_STAT(MATCH_MULHU, ARITH1)
DEFINE_INSN_STAT(MATCH_MULW, ARITH1)
DEFINE_INSN_STAT(MATCH_OR, ARITH0)
DEFINE_INSN_STAT(MATCH_ORI, ARITH0)
DEFINE_INSN_STAT(MATCH_REM, ARITH1)
DEFINE_INSN_STAT(MATCH_REMU, ARITH1)
DEFINE_INSN_STAT(MATCH_REMUW, ARITH1)
DEFINE_INSN_STAT(MATCH_REMW, ARITH1)
DEFINE_INSN_STAT(MATCH_SB, STORE)
DEFINE_INSN_STAT(MATCH_SC_D, STORE)
DEFINE_INSN_STAT(MATCH_SC_W, STORE)
DEFINE_INSN_STAT(MATCH_SD, STORE)
DEFINE_INSN_STAT(MATCH_SFENCE_VMA, CONTROL)
DEFINE_INSN_STAT(MATCH_SH, STORE)
DEFINE_INSN_STAT(MATCH_SLL, ARITH0)
DEFINE_INSN_STAT(MATCH_SLLI, ARITH0)
DEFINE_INSN_STAT(MATCH_SLLIW, ARITH0)
DEFINE_INSN_STAT(MATCH_SLLW, ARITH0)
DEFINE_INSN_STAT(MATCH_SLT, ARITH0)
DEFINE_INSN_STAT(MATCH_SLTI, ARITH0)
DEFINE_INSN_STAT(MATCH_SLTIU, ARITH0)
DEFINE_INSN_STAT(MATCH_SLTU, ARITH0)
DEFINE_INSN_STAT(MATCH_SRA, ARITH0)
DEFINE_INSN_STAT(MATCH_SRAI, ARITH0)
DEFINE_INSN_STAT(MATCH_SRAIW, ARITH0)
DEFINE_INSN_STAT(MATCH_SRAW, ARITH0)
DEFINE_INSN_STAT(MATCH_SRET, CONTROL)
DEFINE_INSN_STAT(MATCH_SRL, ARITH0)
DEFINE_INSN_STAT(MATCH_SRLI, ARITH0)
DEFINE_INSN_STAT(MATCH_SRLIW, ARITH0)
DEFINE_INSN_STAT(MATCH_SRLW, ARITH0)
DEFINE_INSN_STAT(MATCH_SUB, ARITH0)
DEFINE_INSN_STAT(MATCH_SUBW, ARITH0)
DEFINE_INSN_STAT(MATCH_SW, STORE)
DEFINE_INSN_STAT(MATCH_WFI, CONTROL)
DEFINE_INSN_STAT(MATCH_XOR, ARITH0)
DEFINE_INSN_STAT(MATCH_XORI, ARITH0)
DEFINE_INSN_STAT(MATCH_VADD, VARITH0)
DEFINE_INSN_STAT(MATCH_VSETVL, CONTROL)
DEFINE_INSN_STAT(MATCH_VCONFIG, CONTROL)
DEFINE_INSN_STAT(MATCH_VLH, VLOAD)
DEFINE_INSN_STAT(MATCH_VSH, VSTORE)
DEFINE_INSN_STAT(MATCH_VMUL, VARITH1)
DEFINE_INSN_STAT(MATCH_VREDSUM, VARITH1)

#endif