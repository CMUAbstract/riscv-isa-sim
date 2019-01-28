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
	for(uint16_t i = 0, j = 0; i < VL* sizeof(int16_t); i += sizeof(int16_t), j++)
		if((insn.fr() == VMASK_NOMASK) ||
			(insn.fr() == VMASK_LSB && VMP(j)) ||
			(insn.fr() == VMASK_ILSB && !VMP(j))) {
			WRITEP_VRD(MMU.load_int16(RS1 + i), j); 
		} else WRITEP_VRD(0, j);
}