require_extension('V');

#define ONE 0
#define ZERO 1
#define FIRST 2
#define LAST 3
#define ODD 4
#define EVEN 5
#define IDX 6

for(uint8_t i = 0; i < VL; i++) {
	switch(insn.vsimm5()) {
		case ONE: {
			WRITEP_VREG(0x0, 0x1, i);
			break;
		}
		case ZERO: {
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