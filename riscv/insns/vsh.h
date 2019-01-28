require_extension('V');
if(VL == 1 || insn.fr() == VMASK_SCALAR) {
	int16_t v = VS1P(0);
	for(uint16_t i = 0; i < VL; i++) {
		if((insn.fr() == VMASK_NOMASK) ||
			(insn.fr() == VMASK_LSB && VMP(0)) ||
			(insn.fr() == VMASK_ILSB && !VMP(0)) ||
			insn.fr() == VMASK_SCALAR) {
			MMU.store_uint16(RS2 + i * sizeof(int16_t), v);
		} else MMU.store_uint16(RS2 + i * sizeof(int16_t), 0);
	}
} else {
	for(uint16_t i = 0, j = 0; i < VL * sizeof(int16_t); i += sizeof(int16_t), j++) {
		if((insn.fr() == VMASK_NOMASK) ||
			(insn.fr() == VMASK_LSB && VMP(j)) ||
			(insn.fr() == VMASK_ILSB && !VMP(j))) {
			MMU.store_uint16(RS2 + i, VS1P(j));
		} else MMU.store_uint16(RS2 + i, 0);
	}
}
