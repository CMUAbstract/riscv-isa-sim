require_extension('V');
int64_t v = 0;
for(uint8_t i = 0; i < VL; i++) {
	if((insn.vm() && VMP(i)) || !insn.vm()) {
		v = sext_xlen(VS1P(i) + v);
		WRITEP_VRD(v, i);
	}
}
