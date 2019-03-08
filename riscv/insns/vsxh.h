require_extension('V');
for(uint16_t i = 0; i < VL; i++) {
	if((insn.vm() && VMP(i)) || !insn.vm())
		MMU.store_uint16(
			READ_REG(insn.rd()) + sizeof(int16_t) * VS2P(i), VS1P(i));
}