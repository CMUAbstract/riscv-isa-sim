#! /bin/bash

if [ "x$RISCV" = "x" ]
then
  echo "Please set the RISCV environment variable."
  exit 1
fi
PATH="$RISCV/bin:$PATH"

if [ ! `which riscv32-unknown-elf-gcc` ]
then
  echo "riscv32-unknown-elf-gcc doesn't appear to be installed"
  exit 1
fi

echo "Starting RISC-V SPIKE configuration process"

if [ -e "./build" ]
then
  echo "Removing existing ./build directory"
  rm -rf "./build"
fi
if [ ! -e "./configure" ]
then
  (
    cd "."
    find . -iname configure.ac | sed s/configure.ac/m4/ | xargs mkdir -p
    autoreconf -i
  )
fi
mkdir -p "./build"
cd "./build"
echo "Configuring project ."
../configure --prefix=$(pwd)/bin --with-fesvr=$(pwd)/../../fesvr/bin > build.log

echo "Done configuring RISC-V SPIKE"
