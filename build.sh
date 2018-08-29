#! /bin/bash
#
# Script to build RISC-V ISA simulator, proxy kernel, and GNU toolchain.
# Tools will be installed to $RISCV.

. build.common

if [ ! `which riscv32-unknown-elf-gcc` ]
then
  echo "riscv32-unknown-elf-gcc doesn't appear to be installed; use the full-on build.sh"
  exit 1
fi

echo "Starting RISC-V Toolchain build process"

if [ $1 == 1 ]
then
configure_project . --prefix=$(pwd)/bin --with-fesvr=$RISCV --with-isa=rv32ima --enable-histogram --enable-32bit
make_project riscv-isa-sim
else
cd build
make_project riscv-isa-sim
fi

echo -e "\\nRISC-V Toolchain installation completed!"
