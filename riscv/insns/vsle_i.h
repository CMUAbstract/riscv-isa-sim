require_extension('V');
for(uint8_t i = 0; i < VL; i++) {
	if((insn.vm() && VMP(i)) || !insn.vm())
		WRITEP_VRD(sreg_t(VS1P(i)) <= sreg_t(insn.vsimm5()), i);
}
