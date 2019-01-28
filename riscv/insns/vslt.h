require_extension('V');
for(uint8_t i = 0; i < VL; i++) {
	if((insn.fr() == VMASK_NOMASK) ||
		(insn.fr() == VMASK_LSB && VMP(i)) ||
		(insn.fr() == VMASK_ILSB && !VMP(i))) {
		WRITEP_VRD(sreg_t(VS1P(i)) < sreg_t(VS2P(i)), i);
	} else if(insn.fr() == VMASK_SCALAR) {
		WRITEP_VRD(sreg_t(VS1P(0)) < sreg_t(VS2P(0)), i);
	} else WRITEP_VRD(0, i); 
}
