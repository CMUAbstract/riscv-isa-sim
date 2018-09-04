// See LICENSE for license details.

#include "simcall.h"
#include "sim.h"

simcall_t::simcall_t(sim_t *sim)
	: sim(sim), memif(&sim->memif()), table(128) {
	table[12] = &simcall_t::sim_mark;
	table[24] = &simcall_t::sim_clearmark;
	register_command(0, std::bind(
		&simcall_t::handle_simcall, this, std::placeholders::_1), "simcall");
}

void simcall_t::handle_simcall(command_t cmd) {
	reg_t magicmem[8];
	memif->read(cmd.payload(), sizeof(magicmem), magicmem);
	
	reg_t n = magicmem[0];
	if (n > table.size() || !table[n])
		throw std::runtime_error("bad simcall #" + std::to_string(n));

	magicmem[0] = (this->*table[n])(magicmem[1], magicmem[2], magicmem[3], 
		magicmem[4], magicmem[5], magicmem[6], magicmem[7]);
	memif->write(cmd.payload(), sizeof(magicmem), magicmem);	
}

reg_t simcall_t::sim_mark(
	reg_t addr, reg_t size, reg_t a2, reg_t a3, reg_t a4, reg_t a5, reg_t a6) {
	sim->mark(addr, size);
	return 0;
}

reg_t simcall_t::sim_clearmark(
	reg_t addr, reg_t size, reg_t a2, reg_t a3, reg_t a4, reg_t a5, reg_t a6) {
	sim->clear_mark(addr, size);
	return 0;
}