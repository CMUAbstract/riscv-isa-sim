require_extension('V');
if(VL == 1) {
	int16_t v = VS1P(0);
	for(uint16_t i = 0; i < VL; i++)
		MMU.store_uint16(RS2 + i * sizeof(int16_t), v);
} else {
	for(uint16_t i = 0, j = 0; i < VL * sizeof(int16_t); i += sizeof(int16_t), j++)
		MMU.store_uint16(RS2 + i, VS1P(j));
}
