require_extension('V');
for(uint8_t i = 0; i < VL; i++) {
	if((insn.m() && VMP(i)) || !insn.m()) {
		WRITEP_VRD(sext_xlen(VS1P(i) * VS2P(i)), i);
	} else WRITEP_VRD(0, i);
}
