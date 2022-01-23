#!/bin/bash

set -e

rm -rf obj_dir

verilator -trace -cc ../rtl/*.v +1364-2005ext+v --top-module top -Wno-fatal
VERILATOR_ROOT=/usr/share/verilator/
cd obj_dir; make -f Vtop.mk; cd ..
g++ -std=c++14 verilator-main.cpp obj_dir/Vtop__ALL.a -I obj_dir/ -I $VERILATOR_ROOT/include/ -I $VERILATOR_ROOT/include/vltstd $VERILATOR_ROOT/include/verilated.cpp $VERILATOR_ROOT/include/verilated_vcd_c.cpp -I. -o max9850-test -O0 -g3 -ldl -Wl,--export-dynamic
