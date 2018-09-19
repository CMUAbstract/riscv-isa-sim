require_extension('V');
for(uint8_t i = 0, j = 0; i < VL * sizeof(int16_t); i += sizeof(int16_t))
	WRITEP_VRD(MMU.load_int16(RS1 + i), j++);