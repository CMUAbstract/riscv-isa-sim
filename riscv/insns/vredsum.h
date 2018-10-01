require_extension('V');
int64_t v = 0;
for(uint8_t i = 0; i < VL; i++)
	v = sext_xlen(VS1P(i) + v);
for(uint8_t i = 0; i < VL; i++)
	WRITEP_VRD(v, i);