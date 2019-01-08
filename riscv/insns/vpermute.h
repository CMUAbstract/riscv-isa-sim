require_extension('V');
if(VL == 1) {
	int16_t v = VS1P(0);
	for(uint8_t i = 0; i < MAXVL; i++) WRITEP_VRD(v, i);
} else {
	for(uint8_t i = 0; i < VL; i++) {
		assert(VS2P(i) < VL);
		WRITEP_VRD(VS1P(i), VS2P(i));
	}
}