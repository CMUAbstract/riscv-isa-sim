require_extension('V');
for(uint16_t i = 0; i < VL; i++) {
	if((insn.vm() && VMP(i)) || !insn.vm())
		WRITEP_VRD(MMU.load_int32(RS1 + sizeof(int32_t) * VS2P(i)), i);
}