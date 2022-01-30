/*
 * Copyright (C) 2019-2022 Markus Lavin (https://www.zzzconsulting.se/)
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

#include <stdint.h>

#define R_I2C_SCL ((volatile uint32_t *)0x30000004)
#define R_I2C_SDA ((volatile uint32_t *)0x30000008)

void i2c_delay() {
  for (volatile int i = 0; i < 4; i++)
    ;
}

void i2c_start() {
  *R_I2C_SDA = 0;
  i2c_delay();
}

void i2c_stop() {
  *R_I2C_SCL = 0;
  *R_I2C_SDA = 0;
  i2c_delay();
  *R_I2C_SCL = 1;
  i2c_delay();
  *R_I2C_SDA = 1;
  i2c_delay();
}

uint8_t i2c_write_byte(uint8_t byte) {
  for (int i = 0; i < 8; i++) {
    *R_I2C_SCL = 0;
    i2c_delay();
    *R_I2C_SDA = (byte >> (7 - i)) & 1;
    i2c_delay();
    *R_I2C_SCL = 1;
    i2c_delay();
    i2c_delay();
  }
  // Get acknowledge from device
  *R_I2C_SCL = 0;
  *R_I2C_SDA = 1;
  i2c_delay();
  i2c_delay();
  *R_I2C_SCL = 1;
  i2c_delay();
  return *R_I2C_SDA & 1;
}

void max9850_write_reg(uint8_t addr, uint8_t data) {
  i2c_start();
  i2c_write_byte(0x20); // I2C address for write
  i2c_write_byte(addr);
  i2c_write_byte(data);
  i2c_stop();
}

void long_delay() {
  for (volatile int i = 0; i < 50000; i++)
    i2c_delay();
}

int main(void) {
  *R_I2C_SCL = 1;
  *R_I2C_SDA = 1;

  long_delay();

  // Enable (0x5) - Disable everything
  max9850_write_reg(0x05, 0x00);

  // Clock (0x6) - ICLK = MCLK = 12.5MHz
  max9850_write_reg(0x04, 0x00);

  // General Purpose (0x3)
  max9850_write_reg(0x03, 0x00);

  // Digital Audio (0xA) - MAS=0 (slave mode), BCINV=1
  max9850_write_reg(0x0a, 0x20);

  // LRCLK MSB (0x8) - INT=1 (integer mode)
  max9850_write_reg(0x08, 0x80);

  // LRCLK LSB (0x9) - LSB=16
  max9850_write_reg(0x09, 0x10);

  // Charge Pump (0x7) - NC=9
  max9850_write_reg(0x07, 0x09);

  // Interrupt Enable (0x4) - Disable all interrupts
  max9850_write_reg(0x04, 0x00);

  // Volume (0x2)
  max9850_write_reg(0x02, 0x1c);

  // Enable (0x5)
  max9850_write_reg(0x05, 0xfd);

  while (1) {
    // General-purpose register - GPIO outputs low
    max9850_write_reg(0x03, 0x20);

    long_delay();

    // General-purpose register - GPIO outputs high-z
    max9850_write_reg(0x03, 0x60);

    long_delay();
  }

  return 0;
}
