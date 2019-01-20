require_extension('V');
for(uint8_t i = 0; i < VL; i++)
	WRITEP_VRD(sreg_t(VS1P(i)) > sreg_t(VS2P(i)), i);
