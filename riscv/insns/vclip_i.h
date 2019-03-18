require_extension('V');
int16_t F_N = insn.vsimm5();
int16_t F_K = (1 << (F_N - 1));
for(uint8_t i = 0; i < VL; i++) {
	if((insn.vm() && VMP(i)) || !insn.vm())
		WRITEP_VRD(sext_xlen((VS1P(i) + F_K) >> F_N), i);
}
