require_extension('V');
reg_t len = (RS1 > max_vl) ? max_vl : RS1;
WRITE_RD(len);
p->set_csr(CSR_VL, len);