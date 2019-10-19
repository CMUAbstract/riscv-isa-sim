require_extension('V');

#define CLEAR 0
#define FIRST 1
#define LAST 2
#define ODD 3
#define EVEN 4
#define IDX 5

for(uint8_t i = 0; i < VL; i++) {
	switch(insn.vsimm5()) {
		case CLEAR: {
			WRITEP_VREG(0x0, 0x0, i);
			break;
		}
		case FIRST: {
			if(i == 0) WRITEP_VREG(0x0, 0x1, i);
			else WRITEP_VREG(0x0, 0x0, i);
			break;
		}
		case LAST: {
			if(i == VL - 1) WRITEP_VREG(0x0, 0x1, i);
			else WRITEP_VREG(0x0, 0x0, i);
			break;
		}
		case ODD: {
			if(i & 0x1) WRITEP_VREG(0x0, 0x1, i);
			else WRITEP_VREG(0x0, 0x0, i);
			break;
		}
		case EVEN: {
			if(i & 0x1) WRITEP_VREG(0x0, 0x0, i);
			else WRITEP_VREG(0x0, 0x1, i);
			break;
		}
		case IDX: {
			WRITEP_VREG(0x0, i, i);
			break;
		}
		default: break;
	}
}