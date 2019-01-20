require_extension('V');
for(uint8_t i = 0; i < VL; i++)
	WRITEP_VRD(sext_xlen(~VS1P(i)), i);
