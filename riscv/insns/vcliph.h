require_extension('V');
int16_t F_N = RS2;
int16_t F_K = (1 << (F_N - 1));
for(uint16_t i = 0; i < VL; i++)
	WRITEP_VRD(((VS1P(i) + F_K) >> F_N), i);