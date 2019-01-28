require_extension('V');
if(VL == 1 || insn.fr() == VMASK_SCALAR) {
	int16_t v = VS1P(0);
	for(uint8_t i = 0; i < MAXVL; i++) {
		if((insn.fr() == VMASK_NOMASK) ||
			(insn.fr() == VMASK_LSB && VMP(0)) ||
			(insn.fr() == VMASK_ILSB && !VMP(0)) ||
			insn.fr() == VMASK_SCALAR) {
			WRITEP_VRD(v, i);
		} else WRITEP_VRD(0, i);
	}
} else {
	for(uint8_t i = 0; i < VL; i++) {
		assert(VS2P(i) < VL);
		if((insn.fr() == VMASK_NOMASK) ||
			(insn.fr() == VMASK_LSB && VMP(i)) ||
			(insn.fr() == VMASK_ILSB && !VMP(i))) {
			WRITEP_VRD(VS1P(i), VS2P(i));
		} else WRITEP_VRD(0, VS2P(i));
	}
}
