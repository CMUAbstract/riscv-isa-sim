#! /bin/bash

if [ "x$RISCV" = "x" ]
then
  echo "Please set the RISCV environment variable."
  exit 1
fi

PATH="$RISCV/bin:$PATH"
MAKE=`command -v gmake || command -v make`
THREADS=`expr $(cat /proc/cpuinfo | awk '/^processor/{print $3}' | wc -l) \* 2`

if [ ! `which riscv32-unknown-elf-gcc` ]
then
  echo "riscv32-unknown-elf-gcc doesn't appear to be installed"
  exit 1
fi

echo "Starting RISC-V SPIKE build process"
cd build
echo "Building project ."
$MAKE -j$THREADS >> build.log
echo "Installing project ."
$MAKE -j$THREADS install >> build.log
cd - > /dev/null
echo "RISC-V SPIKE build done"
