require_extension('V');
for(uint16_t i = 0; i < VL; i++) {
	if((insn.vm() && VMP(i)) || !insn.vm())
		MMU.store_uint16(RS1 + sizeof(int16_t) * VS3P(i), VS2P(i));
}