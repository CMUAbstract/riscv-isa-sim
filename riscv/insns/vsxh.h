require_extension('V');
if(VL == 1 || insn.fr() == VMASK_SCALAR) {
	int16_t v = VS1P(0);
	for(uint16_t i = 0; i < VL; i++) {
		if((insn.fr() == VMASK_NOMASK) ||
			(insn.fr() == VMASK_LSB && VMP(0)) ||
			(insn.fr() == VMASK_ILSB && !VMP(0)) ||
			insn.fr() == VMASK_SCALAR) {
			MMU.store_uint16(
				READ_REG(insn.rd()) + sizeof(int16_t) * VS2P(i), v);
		} else MMU.store_uint16(
			READ_REG(insn.rd()) + sizeof(int16_t) * VS2P(i), 0);
	}
} else {
	for(uint16_t i = 0; i < VL; i++) {
		if((insn.fr() == VMASK_NOMASK) ||
			(insn.fr() == VMASK_LSB && VMP(i)) ||
			(insn.fr() == VMASK_ILSB && !VMP(i))) { 
			MMU.store_uint16(
				READ_REG(insn.rd()) + sizeof(int16_t) * VS2P(i), VS1P(i));
		} else MMU.store_uint16(
			READ_REG(insn.rd()) + sizeof(int16_t) * VS2P(i), 0);
	}
}