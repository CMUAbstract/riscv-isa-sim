require_extension('V');
for(uint16_t i = 0, j = 0; i < VL * sizeof(int16_t); i += sizeof(int16_t), j++) {
	if((insn.vm() && VMP(j)) || !insn.vm())
		WRITEP_VRD(MMU.load_int16(RS1 + i * RS2), j);
}