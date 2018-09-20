require_extension('V');
if(VL == 1) {
	int16_t v = MMU.load_int16(RS1);
	for(uint8_t i = 0; i < MAXVL; i++) WRITEP_VRD(v, i);
} else {
	for(uint8_t i = 0, j = 0; i < VL * sizeof(int16_t); i += sizeof(int16_t))
		WRITEP_VRD(MMU.load_int16(RS1 + i), j++);
}