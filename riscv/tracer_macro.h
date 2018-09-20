#undef READ_REG
#undef READ_FREG
#undef READ_VREG
#undef READP_VREG
#undef WRITE_RD
#undef WRITE_REG
#undef WRITE_FREG
#undef WRITE_VREG
#undef WRITEP_VREG
#define READ_REG(reg) STATE.XPR[ws.log_input_reg(reg)]
#define READ_FREG(reg) STATE.FPR[ws.log_input_freg(reg)]
#define WRITE_RD(value) WRITE_REG(ws.log_output_reg(insn.rd()), value)

#ifndef RISCV_ENABLE_COMMITLOG
# define WRITE_REG(reg, value) STATE.XPR.write(ws.log_output_reg(reg), value)
# define WRITE_FREG(reg, value) DO_WRITE_FREG(ws.log_output_freg(reg), freg(value))
#else
# define WRITE_REG(reg, value) ({ \
    reg_t wdata = (value); /* value may have side effects */ \
    STATE.log_reg_write = (commit_log_reg_t){(reg) << 1, {wdata, 0}}; \
    STATE.XPR.write(ws.log_output_reg(reg), wdata); \
  })
# define WRITE_FREG(reg, value) ({ \
    freg_t wdata = freg(value); /* value may have side effects */ \
    STATE.log_reg_write = (commit_log_reg_t){((reg) << 1) | 1, wdata}; \
    DO_WRITE_FREG(ws.log_output_freg(reg), wdata); \
  })
#endif

#define READ_VREG(reg) STATE.VPR[ws.log_input_vreg(reg)]
#define READP_VREG(reg, pos) STATE.VPR.read(ws.log_input_vreg(reg), pos)
#define WRITE_VREG(reg, value) STATE.VPR.write(ws.log_output_vreg(reg), value)
#define WRITEP_VREG(reg, value, pos) STATE.VPR.write(ws.log_output_vreg(reg), value, pos)

#define load_uint8(addr) load_uint8(ws.log_input_loc(addr))
#define load_uint16(addr) load_uint16(ws.log_input_loc(addr))
#define load_uint32(addr) load_uint32(ws.log_input_loc(addr))
#define load_uint64(addr) load_uint64(ws.log_input_loc(addr))
#define load_int8(addr) load_int8(ws.log_input_loc(addr))
#define load_int16(addr) load_int16(ws.log_input_loc(addr))
#define load_int32(addr) load_int32(ws.log_input_loc(addr))
#define load_int64(addr) load_int64(ws.log_input_loc(addr))

#define store_uint8(addr, val) store_uint8(ws.log_output_loc(addr), val)
#define store_uint16(addr, val) store_uint16(ws.log_output_loc(addr), val)
#define store_uint32(addr, val) store_uint32(ws.log_output_loc(addr), val)
#define store_uint64(addr, val) store_uint64(ws.log_output_loc(addr), val)
#define store_int8(addr, val) store_int8(ws.log_output_loc(addr), val)
#define store_int16(addr, val) store_int16(ws.log_output_loc(addr), val)
#define store_int32(addr, val) store_int32(ws.log_output_loc(addr), val)
#define store_int64(addr, val) store_int64(ws.log_output_loc(addr), val)