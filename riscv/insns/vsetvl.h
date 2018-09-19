require_extension('V');
reg_t vregmax = p->get_csr(CSR_VREGMAX);
if(vregmax != 0) {
	reg_t vemaxw = get_field(vregmax, MVEC_VEMAXW);
	reg_t maxvl = MAXVL / (1 << (vemaxw - 1));
	reg_t len = (RS1 > maxvl) ? maxvl : RS1;
	WRITE_RD(len);
	p->set_csr(CSR_VL, len);
} else {
	WRITE_RD(0);
}