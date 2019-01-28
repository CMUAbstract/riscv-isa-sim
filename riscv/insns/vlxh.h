require_extension('V');
if(VL == 1 || insn.fr() == VMASK_SCALAR) {
	int16_t v = MMU.load_int16(RS1);
	for(uint16_t i = 0; i < MAXVL; i++) 
		if((insn.fr() == VMASK_NOMASK) ||
			(insn.fr() == VMASK_LSB && VMP(0)) ||
			(insn.fr() == VMASK_ILSB && !VMP(0)) ||
			insn.fr() == VMASK_SCALAR) {
			WRITEP_VRD(v, i);
		} else WRITEP_VRD(0, i);
} else {
	for(uint16_t i = 0; i < VL; i++) {
		if((insn.fr() == VMASK_NOMASK) ||
			(insn.fr() == VMASK_LSB && VMP(i)) ||
			(insn.fr() == VMASK_ILSB && !VMP(i))) {
			WRITEP_VRD(MMU.load_int16(RS1 + sizeof(int16_t) * VS2P(i)), i);
		} else WRITEP_VRD(0, i);
	}
}