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

static const float load_energy = 0.4;
static const float store_energy = 0.4;
static const float control_energy = 0.4;
static const float arith0_energy = 0.4;
static const float arith1_energy = 0.4;
static const float arith2_energy = 0.4;
static const float arith3_energy = 0.4;

#endif