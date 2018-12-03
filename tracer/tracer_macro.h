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
#define WRITE_RD(value) WRITE_REG(ws.log_output_reg(insn.rd(), value), value)

#ifndef RISCV_ENABLE_COMMITLOG
# define WRITE_REG(reg, value) ({												\
		auto wdata = value;														\
		STATE.XPR.write(														\
			ws.log_output_reg(reg, READ_REG(reg)), wdata);						\
	})
# define WRITE_FREG(reg, value) ({												\
		freg_t wdata = freg(value);												\
		DO_WRITE_FREG(ws.log_output_freg(reg, READ_FREG(reg)), wdata);			\
	})
#else
# define WRITE_REG(reg, value) ({ 												\
    reg_t wdata = (value); /* value may have side effects */ 					\
    STATE.log_reg_write = (commit_log_reg_t){(reg) << 1, {wdata, 0}}; 			\
    STATE.XPR.write(ws.log_output_reg(reg, READ_REG(reg)), wdata); 				\
  })
# define WRITE_FREG(reg, value) ({ 												\
    freg_t wdata = freg(value); /* value may have side effects */ 				\
    STATE.log_reg_write = (commit_log_reg_t){((reg) << 1) | 1, wdata}; 			\
    DO_WRITE_FREG(ws.log_output_freg(reg, READ_FREG(reg)), wdata); 				\
  })
#endif

#define READ_VREG(reg) STATE.VPR[ws.log_input_vreg(reg)]
#define READP_VREG(reg, pos) STATE.VPR.read(ws.log_input_vreg(reg), pos)
#define WRITE_VREG(reg, value) ({												\
		auto wdata = value;														\
		STATE.VPR.write(														\
			ws.log_output_vreg(reg, READ_VREG(reg)), wdata);					\
	})
#define WRITEP_VREG(reg, value, pos) ({											\
		auto wdata = value;														\
		auto wpos = pos;														\
		STATE.VPR.write(														\
			ws.log_output_vreg(reg, wdata, READP_VREG(reg, pos)), wdata, wpos);	\
	})

#define load_uint8(addr) load_uint8(ws.log_input_loc<uint8_t>(addr))
#define load_uint16(addr) load_uint16(ws.log_input_loc<uint16_t>(addr))
#define load_uint32(addr) load_uint32(ws.log_input_loc<uint32_t>(addr))
#define load_uint64(addr) load_uint64(ws.log_input_loc<uint64_t>(addr))
#define load_int8(addr) load_int8(ws.log_input_loc<int8_t>(addr))
#define load_int16(addr) load_int16(ws.log_input_loc<int16_t>(addr))
#define load_int32(addr) load_int32(ws.log_input_loc<int32_t>(addr))
#define load_int64(addr) load_int64(ws.log_input_loc<int64_t>(addr))

#define store_uint8(addr, val) store_uint8(										\
	ws.log_output_loc<uint8_t>(addr, MMU.load_uint8(addr)), val);
#define store_uint16(addr, val) store_uint16(									\
	ws.log_output_loc<uint16_t>(addr, MMU.load_uint16(addr)), val);
#define store_uint32(addr, val) store_uint32(									\
	ws.log_output_loc<uint32_t>(addr, MMU.load_uint32(addr)), val);
#define store_uint64(addr, val) store_uint64(									\
	ws.log_output_loc<uint64_t>(addr, MMU.load_uint64(addr)), val);
#define store_int8(addr, val) store_int8(										\
	ws.log_output_loc<int8_t>(addr, MMU.load_int8(addr)), val);
#define store_int16(addr, val) store_int16(										\
	ws.log_output_loc<int16_t>(addr, MMU.load_int16(addr)), val);
#define store_int32(addr, val) store_int32(										\
	ws.log_output_loc<int32_t>(addr, MMU.load_int32(addr)), val);
#define store_int64(addr, val) store_int64(										\
	ws.log_output_loc<int64_t>(addr, MMU.load_int64(addr)), val);

#define get_csr(reg) get_csr(ws.log_input_csr(reg))
#define set_csr(reg, value) set_csr(ws.log_output_csr(reg, p->get_csr(reg), value), value)

#undef set_pc
#define set_pc(x)																\
	do {																		\
		p->check_pc_alignment(x);												\
		npc = ws.log_next_pc(sext_xlen(x));										\
	} while (0)

