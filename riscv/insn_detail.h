#ifndef INSN_DETAIL
#define INSN_DETAIL

#define LOAD 0x01
#define STORE 0x02
#define CONTROL 0x03
#define ARITH0 0x04
#define ARITH1 0x05
#define ARITH2 0x06
#define ARITH3 0x07
#define VLOAD 0x08
#define VSTORE 0x09
#define VARITH0 0x10
#define VARITH1 0x11

static const uint16_t load_cycles = 1;
static const uint16_t store_cycles = 1;
static const uint16_t control_cycles = 1;
static const uint16_t arith0_cycles = 1;
static const uint16_t arith1_cycles = 2;
static const uint16_t arith2_cycles = 2;
static const uint16_t arith3_cycles = 3;
static const uint16_t vamortization = 2;
static const uint16_t vload_cycles = 1;
static const uint16_t vstore_cycles = 1;
static const uint16_t varith0_cycles = 1;
static const uint16_t varith1_cycles = 2;

static const double load_energy = 80e-9;
static const double store_energy = 110e-9;
static const double control_energy = 70.03e-9;
static const double arith0_energy = 70.03e-9;
static const double arith1_energy = 71e-9;
static const double arith2_energy = 71e-9;
static const double arith3_energy = 74e-9;
static const double vload_energy = 10e-9;
static const double vstore_energy = 40e-9;
static const double varith0_energy = 0.03e-9;
static const double varith1_energy = 1e-9;

#endif