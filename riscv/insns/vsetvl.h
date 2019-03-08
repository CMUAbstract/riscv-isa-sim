require_extension('V');
reg_t len = (RS1 > MAXVL) ? MAXVL : RS1;
WRITE_RD(len);
p->set_csr(CSR_VL, len);