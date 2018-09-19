require_extension('V');
for(uint8_t i = 0, j = 0; i < VL * sizeof(int16_t); i += sizeof(int16_t))
	MMU.store_uint16(RS2 + i, VS1P(j++));
