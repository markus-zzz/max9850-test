#!/bin/bash

# exit when any command fails
set -e

clang --target=riscv32 -march=rv32ic -std=c99 -O3 -mno-relax -Wall -Werror max9850-test.c -c
llvm-mc --arch=riscv32 -mcpu=generic-rv32 -mattr=+c -assemble start.S --filetype=obj -o start.o
ld.lld -T system.ld start.o max9850-test.o -o max9850-test.elf
llvm-objcopy --only-section=.text --output-target=binary max9850-test.elf max9850-test.bin
hexdump -v -e '4/4 "%08x " "\n"' max9850-test.bin > rom.vh
