require_extension('V');
for(uint8_t i = 0; i < VL; i++) {
	assert(VS2P(i) < VL);
	if((insn.vm() && VMP(i)) || !insn.vm())
		WRITEP_VRD(VS1P(i), VS2P(i));
}
