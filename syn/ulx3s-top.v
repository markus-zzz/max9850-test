/*
 * Copyright (C) 2019-2020 Markus Lavin (https://www.zzzconsulting.se/)
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

`default_nettype none

/*
    -    +
27 SDA   SCL
26 LRCLK
25 BCLK
24 SDIN
23 MCLK
22
21 GPIO
*/

module ulx3s_top(
  input clk_25mhz,
  input [6:0] btn,
  output [7:0] led,
  inout [27:0] gp,
  inout [27:0] gn,
  inout usb_fpga_bd_dp,
  inout usb_fpga_bd_dn,
  output usb_fpga_pu_dp,
  output usb_fpga_pu_dn
);



  wire [31:0] o_port_0;
  wire [31:0] o_port_1;
  wire [31:0] o_port_2;

  wire [31:0] i_port_0;
  wire [31:0] i_port_1;
  wire [31:0] i_port_2;

  //assign led = port_0[7:0];
  assign led = {3'b000, gn[21], 4'b0000};

  // SCL
  assign gp[27] = o_port_1[0] ? 1'bz : 1'b0;
  assign i_port_1[0] = gp[27];
  // SDA
  assign gn[27] = o_port_2[0] ? 1'bz : 1'b0;
  assign i_port_2[0] = gn[27];

  top u_top(
    .i_rst(btn[6]),
    .i_clk(clk_25mhz),

    .o_port_0(o_port_0),
    .o_port_1(o_port_1),
    .o_port_2(o_port_2),

    .i_port_0(i_port_0),
    .i_port_1(i_port_1),
    .i_port_2(i_port_2),

    .tonesel(btn[5]),

    .LRCLK(gn[26]),
    .BCLK(gn[25]),
    .SDIN(gn[24]),
    .MCLK(gn[23])
  );

endmodule
