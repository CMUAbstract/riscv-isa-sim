require_extension('V');
for(uint16_t i = 0; i < VL; i++) {
	if((insn.vm() && VMP(i)) || !insn.vm())
		WRITEP_VRD(MMU.load_int16(RS1 + sizeof(int16_t) * VS2P(i)), i);
}