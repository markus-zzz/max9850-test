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

/* verilator lint_off WIDTH */
/* verilator lint_off PINMISSING */

`default_nettype none

module top(
  input i_rst,
  input i_clk,

  output reg [31:0] o_port_0,
  output reg [31:0] o_port_1,
  output reg [31:0] o_port_2,

  input [31:0] i_port_0,
  input [31:0] i_port_1,
  input [31:0] i_port_2,

	output wire I2C_SCL,
	output wire I2C_SDA_O,
	output wire I2C_SDA_OE,
	input wire  I2C_SDA_I
);

  wire cpu_mem_valid;
  wire cpu_mem_instr;
  wire cpu_mem_ready;
  wire [31:0] cpu_mem_addr;
  wire [31:0] cpu_mem_wdata;
  wire [ 3:0] cpu_mem_wstrb;
  reg [31:0] cpu_mem_rdata;
  wire [31:0] ram_rdata, rom_rdata;

  wire [9:0] ram_addr;
  wire [31:0] ram_wdata;
  wire [3:0] ram_wstrb;

  wire hyp_rd_req;
  wire hyp_wr_req;
  wire hyp_rd_rdy;
  wire hyp_busy;
  wire [31:0] hyp_rdata;
  reg [31:0] hyp_rdata_r;


  assign ram_addr  = cpu_mem_addr[9:0];
  assign ram_wdata = cpu_mem_wdata;
  assign ram_wstrb = cpu_mem_wstrb;

  always @(posedge clk) begin
    if (rst)
      hyp_rdata_r <= 0;
    else if (hyp_rd_rdy)
      hyp_rdata_r <= hyp_rdata;
  end

  always @* begin
    case (cpu_mem_addr[31:28])
      4'h0: cpu_mem_rdata = rom_rdata;
      4'h1: cpu_mem_rdata = ram_rdata;
      4'h3: begin
        case (cpu_mem_addr[3:0])
          4'h0: cpu_mem_rdata = i_port_0;
          4'h4: cpu_mem_rdata = i_port_1;
          4'h8: cpu_mem_rdata = i_port_2;
          default: cpu_mem_rdata = 0;
        endcase
      end
      4'h5: cpu_mem_rdata = i2c_status_reg;
      default: cpu_mem_rdata = 0;
    endcase
  end

  wire clk, rst;
  assign clk = i_clk;
  assign rst = i_rst;

  parameter S_IDLE      = 3'd0,
            S_CPU_READY = 3'd1;

  reg [2:0] fsm_state;

  always @(posedge clk) begin
    if (rst) begin
      fsm_state <= S_IDLE;
    end
    else begin
      case (fsm_state)
      S_IDLE: begin
        if (cpu_mem_valid) begin
          case (cpu_mem_addr[31:28])
          4'h0: fsm_state <= S_CPU_READY; // ROM
          4'h1: fsm_state <= S_CPU_READY; // RAM
          4'h3: fsm_state <= S_CPU_READY; // GPIO
          4'h5: fsm_state <= S_CPU_READY; // I2C
          default: /* do nothing */;
          endcase
        end
      end
      S_CPU_READY: begin
        fsm_state <= S_IDLE;
      end
      default: /* do nothing */;
      endcase
    end
  end

  assign cpu_mem_ready = (fsm_state == S_CPU_READY);

  // ROM - CPU code.
  sprom #(
    .aw(10),
    .dw(32),
    .MEM_INIT_FILE("rom.vh")
  ) u_rom(
    .clk(clk),
    .rst(rst),
    .ce(cpu_mem_valid && cpu_mem_addr[31:28] == 4'h0),
    .oe(1'b1),
    .addr(cpu_mem_addr[11:2]),
    .do(rom_rdata)
  );

  // RAM - shared between CPU and USB. USB has priority.
  genvar gi;
  generate
    for (gi=0; gi<4; gi=gi+1) begin
      spram #(
        .aw(10),
        .dw(8)
      ) u_ram(
        .clk(clk),
        .rst(rst),
        .ce(cpu_mem_valid && cpu_mem_addr[31:28] == 4'h1),
        .oe(1'b1),
        .addr(ram_addr[9:2]),
        .do(ram_rdata[(gi+1)*8-1:gi*8]),
        .di(ram_wdata[(gi+1)*8-1:gi*8]),
        .we(ram_wstrb[gi])
      );
    end
  endgenerate

  picorv32 #(
    .COMPRESSED_ISA(1)
  ) u_cpu(
    .clk(clk),
    .resetn(~rst),
    .mem_valid(cpu_mem_valid),
    .mem_instr(cpu_mem_instr),
    .mem_ready(cpu_mem_ready),
    .mem_addr(cpu_mem_addr),
    .mem_wdata(cpu_mem_wdata),
    .mem_wstrb(cpu_mem_wstrb),
    .mem_rdata(cpu_mem_rdata)
  );

  always @(posedge clk) begin
    if (rst) begin
      o_port_0 <= 0;
      o_port_1 <= 0;
      o_port_2 <= 0;
    end
    else if (cpu_mem_wstrb == 4'hf && cpu_mem_addr[31:28] == 4'h3) begin
      case (cpu_mem_addr[3:0])
        4'h0: o_port_0 <= cpu_mem_wdata;
        4'h4: o_port_1 <= cpu_mem_wdata;
        4'h8: o_port_2 <= cpu_mem_wdata;
        default: /* do nothing */;
      endcase
    end
  end

	wire i2c_cmd_pulse;
	wire i2c_irq_ack_pulse;
	reg [10:0] i2c_ctrl_reg;
	wire [9:0] i2c_status_reg;

`ifdef USE_I2C_CONTROLLER_BLOCK
  always @(posedge clk) begin
    if (rst) begin
      i2c_ctrl_reg <= 0;
    end
    else if (cpu_mem_valid && cpu_mem_wstrb == 4'hf && cpu_mem_addr[31:28] == 4'h5) begin
      case (cpu_mem_addr[3:0])
        4'h0: i2c_ctrl_reg <= cpu_mem_wdata;
        default: /* do nothing */;
      endcase
    end
  end

  assign i2c_cmd_pulse = cpu_mem_valid && cpu_mem_wstrb == 4'hf && cpu_mem_addr[31:28] == 4'h5;

	i2c_controller # (
	  .C_CLK_DIVIDER_LOG2(2))
	u_i2c_controller (
	  .clk(clk),
	  .rst(rst),

	  .i2c_cmd_pulse_i(i2c_cmd_pulse),
	  .i2c_ctrl_reg_i(i2c_ctrl_reg),
	  .i2c_status_reg_o(i2c_status_reg),
	  .i2c_irq_ack_pulse_i(1'b1),
	  .i2c_irq_o(/* NC */),


	  .I2C_SCL(I2C_SCL),
	  .I2C_SDA_O(I2C_SDA_O),
	  .I2C_SDA_OE(I2C_SDA_OE),
	  .I2C_SDA_I(I2C_SDA_I)
	);
`endif

endmodule
