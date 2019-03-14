require_extension('V');
for(uint8_t i = 0; i < VL; i++) {
	if((insn.vm() && VMP(i)) || !insn.vm())
		WRITEP_VRD(sext_xlen(VS1P(i) ^ insn.vsimm5()), i);
}
