require_extension('V');
for(uint16_t i = 0, j = 0; i < VL * sizeof(int32_t); i += sizeof(int32_t), j++) {
	if((insn.vm() && VMP(j)) || !insn.vm())
		WRITEP_VRD(MMU.load_int32(RS1 + i), j);
}