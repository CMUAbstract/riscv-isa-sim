require_extension('V');
if(VL == 1) {
	int16_t v = VS1P(0);
	for(uint8_t i = 0; i < VL * sizeof(int16_t); i += sizeof(int16_t))
		MMU.store_uint16(RS2 + i, v);
} else {
	for(uint8_t i = 0, j = 0; i < VL * sizeof(int16_t); i += sizeof(int16_t))
		MMU.store_uint16(RS2 + i, VS1P(j++));
}
