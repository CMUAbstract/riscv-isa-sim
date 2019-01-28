require_extension('V');
int16_t F_N = RS2;
int16_t F_K = (1 << (F_N - 1));
for(uint8_t i = 0; i < VL; i++) {
	if((insn.fr() == VMASK_NOMASK) ||
		(insn.fr() == VMASK_LSB && VMP(i)) ||
		(insn.fr() == VMASK_ILSB && !VMP(i))) {
		WRITEP_VRD(sext_xlen((VS1P(i) + F_K) >> F_N), i);
	} else if(insn.fr() == VMASK_SCALAR) {
		WRITEP_VRD(sext_xlen((VS1P(0) + F_K) >> F_N), i);
	} else WRITEP_VRD(0, i); 
}
