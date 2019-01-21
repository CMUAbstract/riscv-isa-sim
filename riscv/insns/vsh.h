require_extension('V');
if(VL == 1) {
	int16_t v = VS1P(0);
	for(uint16_t i = 0; i < VL; i++) {
		if((insn.m() && VMP(i)) || !insn.m()) { 
			MMU.store_uint16(RS2 + i * sizeof(int16_t), v);
		} else MMU.store_uint16(RS2 + i * sizeof(int16_t), 0);
	}
} else {
	for(uint16_t i = 0, j = 0; i < VL * sizeof(int16_t); i += sizeof(int16_t), j++) {
		if((insn.m() && VMP(j)) || !insn.m()) {
			MMU.store_uint16(RS2 + i, VS1P(j));
		} else MMU.store_uint16(RS2 + i, 0);
	}
}
