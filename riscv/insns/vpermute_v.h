require_extension('V');
for(uint8_t i = 0; i < VL; i++) {
	if((insn.vm() && VMP(i)) || !insn.vm()) {
		assert(VS2P(i) < VL);
		WRITEP_VRD(VS1P(i), VS2P(i));
	}
}
