`timescale 1 ns / 1 ps

`default_nettype none

module tb;
  reg clk;
  reg rst;

  wire i2c_scl;
  wire i2c_sda_io;
  wire i2c_sda_o;
  wire i2c_sda_oe;
  wire i2c_sda_i;

  top u_top(
    .i_rst(rst),
    .i_clk(clk),

    .o_port_0(),
    .o_port_1(),
    .o_port_2(),

    .I2C_SCL(i2c_scl),
    .I2C_SDA_O(i2c_sda_o),
    .I2C_SDA_OE(i2c_sda_oe),
    .I2C_SDA_I(i2c_sda_i)
  );


	i2c_slave_model u_i2c_slave(
	  .scl(i2c_scl),
	  .sda(i2c_sda_io)
	);

  assign i2c_sda_i = i2c_sda_io;
  assign i2c_sda_io = i2c_sda_oe ? i2c_sda_o : 1'bz;

	// I2C bus needs pullups on both SCL and SDA for correct operation
	assign (weak0, weak1) i2c_scl = 1'b1;
	assign (weak0, weak1) i2c_sda_io = 1'b1;

  initial begin
    $dumpvars;
    clk = 0;
    rst = 1;
    #200 rst = 0;
    #200000 $finish;
  end

  always #40 clk <= ~clk;

endmodule
