require_extension('V');
for(uint8_t i = 0; i < VL; i++)
	if((VS2P(i) & 0x1) == 0x1) WRITEP_VRD(VS1P(i), i); else WRITEP_VRD(0, i);
