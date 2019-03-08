require_extension('V');
for(uint16_t i = 0, j = 0; i < VL * sizeof(int16_t); i += sizeof(int16_t), j++) {
	if((insn.vm() && VMP(j)) || !insn.vm())
		MMU.store_uint16(READ_REG(insn.rd()) + i * RS2, VS1P(j));
}
