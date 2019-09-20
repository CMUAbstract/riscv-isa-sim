require_extension('V');
for(uint16_t i = 0, j = 0; i < VL * sizeof(int32_t); i += sizeof(int32_t), j++) {
	if((insn.vm() && VMP(j)) || !insn.vm())
		MMU.store_uint32(RS1 + i, VS3P(j));
}
