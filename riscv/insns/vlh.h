require_extension('V');
if(VL == 1) {
	int16_t v = MMU.load_int16(RS1);
	for(uint16_t i = 0; i < MAXVL; i++) 
		if((insn.m() && VMP(i)) || !insn.m()) WRITEP_VRD(v, i); 
		else WRITEP_VRD(0, i);
} else {
	for(uint16_t i = 0, j = 0; i < VL* sizeof(int16_t); i += sizeof(int16_t), j++)
		if((insn.m() && VMP(j)) || !insn.m()) WRITEP_VRD(MMU.load_int16(RS1 + i), j); 
		else WRITEP_VRD(0, j);
}