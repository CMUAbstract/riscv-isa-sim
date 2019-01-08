require_extension('V');
if(VL == 1) {
	int16_t v = VS1P(0);
	for(uint16_t i = 0; i < VL * sizeof(int16_t); i += sizeof(int16_t))
		MMU.store_uint16(READ_REG(insn.rd()) + i * RS2, v);
} else {
	for(uint16_t i = 0, j = 0; i < VL * sizeof(int16_t); i += sizeof(int16_t), j++)
		MMU.store_uint16(READ_REG(insn.rd()) + i * RS2, VS1P(j));
}
