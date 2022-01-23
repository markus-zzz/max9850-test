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

#define R_PORT_0 ((volatile uint32_t *)0x30000000)
#define R_PORT_1 ((volatile uint32_t *)0x30000004)
#define R_PORT_2 ((volatile uint32_t *)0x30000008)

#define R_I2C_CTRL ((volatile uint32_t *)0x50000000)
#define R_I2C_STATUS ((volatile uint32_t *)0x50000000)

#define R_I2C_SCL ((volatile uint32_t *)0x30000004)
#define R_I2C_SDA ((volatile uint32_t *)0x30000008)

#define assert(x)                                                              \
  if (!(x)) {                                                                  \
    uint32_t toggle = 0x55555555;                                              \
    while (1) {                                                                \
      *R_PORT_0 = toggle;                                                      \
      toggle = ~toggle;                                                        \
    }                                                                          \
  }

#if 0
const uint32_t i2c_ctrl_we_bit = 1 << 10;
const uint32_t i2c_ctrl_start_bit = 1 << 9;
const uint32_t i2c_ctrl_stop_bit = 1 << 8;

const uint32_t i2c_status_busy_bit = 1 << 9;
const uint32_t i2c_status_ack_bit = 1 << 8;

void i2c_mem_write(uint8_t i2c_addr, uint8_t mem_addr, uint8_t mem_data) {
  uint32_t status;
  /* Make sure interface is not busy */
  while (*R_I2C_STATUS & i2c_status_busy_bit)
    ;

  /* Address for write mode */
  *R_I2C_CTRL = (i2c_ctrl_we_bit | i2c_ctrl_start_bit | i2c_addr << 1 | 0 << 0);

  /* Wait until complete */
  while ((status = *R_I2C_STATUS) & i2c_status_busy_bit)
    ;
  assert(status & i2c_status_ack_bit && "I2C address ACK");

  /* Memory address */
  *R_I2C_CTRL = (i2c_ctrl_we_bit | mem_addr);

  /* Wait until complete */
  while ((status = *R_I2C_STATUS) & i2c_status_busy_bit)
    ;
  assert(status & i2c_status_ack_bit && "MEM address ACK");

  /* Memory data */
  *R_I2C_CTRL = (i2c_ctrl_we_bit | i2c_ctrl_stop_bit | mem_data);

  /* Wait until complete */
  while ((status = *R_I2C_STATUS) & i2c_status_busy_bit)
    ;
  assert(status & i2c_status_ack_bit && "MEM write ACK");
}

uint8_t i2c_mem_read(uint8_t i2c_addr, uint8_t mem_addr) {
  uint32_t status;
  /* Make sure interface is not busy */
  while (*R_I2C_STATUS & i2c_status_busy_bit)
    ;

  /* Address for write mode */
  *R_I2C_CTRL = (i2c_ctrl_we_bit | i2c_ctrl_start_bit | i2c_addr << 1 | 0 << 0);

  /* Wait until complete */
  while ((status = *R_I2C_STATUS) & i2c_status_busy_bit)
    ;
  assert(status & i2c_status_ack_bit && "I2C (write) address ACK");

  /* Memory address */
  *R_I2C_CTRL = (i2c_ctrl_we_bit | mem_addr);

  /* Wait until complete */
  while ((status = *R_I2C_STATUS) & i2c_status_busy_bit)
    ;
  assert(status & i2c_status_ack_bit && "MEM address ACK");

  /* Address for read mode */
  *R_I2C_CTRL = (i2c_ctrl_we_bit | i2c_ctrl_start_bit | i2c_addr << 1 | 1 << 0);

  /* Wait until complete */
  while ((status = *R_I2C_STATUS) & i2c_status_busy_bit)
    ;
  assert(status & i2c_status_ack_bit && "I2C (read) address ACK");

  /* Memory data */
  *R_I2C_CTRL = (i2c_ctrl_stop_bit);

  /* Wait until complete */
  while ((status = *R_I2C_STATUS) & i2c_status_busy_bit)
    ;
  assert(status & i2c_status_ack_bit && "MEM read ACK");

  return status & 0xff;
}

int main(void) {
  /* begin - test */
  /* I2C slave model has address 7'b001_0000 */
#define I2C_ADDR 0x10
#define DATA_SIZE 16
  uint8_t data[DATA_SIZE];

  /* Generate reference data */
  for (int i = 0; i < DATA_SIZE; i++) {
    data[i] = i + 5;
  }

  /* Write data to memory in forward order */
  for (int i = 0; i < DATA_SIZE; i++) {
    i2c_mem_write(I2C_ADDR, i, data[i]);
  }

  /* Read data from memory (and verify) in forward order */
  for (int i = 0; i < DATA_SIZE; i++) {
    assert(i2c_mem_read(I2C_ADDR, i) == data[i]);
  }

  /* Read data from memory (and verify) in reverse order */
  for (int i = 0; i < DATA_SIZE; i++) {
    assert(i2c_mem_read(I2C_ADDR, DATA_SIZE - 1 - i) ==
           data[DATA_SIZE - 1 - i]);
  }

  /* end - test */

  return 0;
}
#else
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

int main(void) {
  *R_I2C_SCL = 1;
  *R_I2C_SDA = 1;

  i2c_delay();
  i2c_delay();
  i2c_delay();

  while (1) {
    i2c_start();
    i2c_write_byte(0x20); // I2C address for write
    i2c_write_byte(0x03); // General-purpose register
    i2c_write_byte(0x20); // General-purpose register - GPIO outputs low
    i2c_stop();

    for (volatile int i = 0; i < 50000; i++)
      i2c_delay();

    i2c_start();
    i2c_write_byte(0x20); // I2C address for write
    i2c_write_byte(0x03); // General-purpose register
    i2c_write_byte(0x60); // General-purpose register - GPIO outputs high-z
    i2c_stop();

    for (volatile int i = 0; i < 50000; i++)
      i2c_delay();
  }

  return 0;
}
#endif
