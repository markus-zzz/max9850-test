/*
 * Copyright (C) 2019-2021 Markus Lavin (https://www.zzzconsulting.se/)
 *
 * All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "Vtop.h"
#include "verilated.h"
#include "verilated_vcd_c.h"
#include <dlfcn.h>
#include <iomanip>
#include <stdio.h>

static Vtop *u_top = NULL;
static VerilatedVcdC *trace = NULL;
static unsigned tick = 0;

double sc_time_stamp() { return tick; }

void applyClk() {
    u_top->i_clk = 1;
    u_top->eval();
    if (trace)
      trace->dump(tick++);
    u_top->i_clk = 0;
    u_top->eval();
    if (trace)
      trace->dump(tick++);
}

int main(int argc, char *argv[]) {
  // Initialize Verilators variables
  Verilated::commandArgs(argc, argv);

  Verilated::traceEverOn(true);

  u_top = new Vtop;

  if (true) {
    trace = new VerilatedVcdC;
    u_top->trace(trace, 99);
    trace->open("dump.vcd");
  }

  u_top->i_clk = 0;
  u_top->i_rst = 0;


  u_top->i_rst = 1;
  applyClk();
  applyClk();
  applyClk();
  u_top->i_rst = 0;

  for (int i = 0; i < 32*1024; i++)
    applyClk();

  trace->flush();

  return 0;
}
