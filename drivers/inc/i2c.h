#pragma once

#include "base.h"
#include "common.h"

#define C_I2CEN (1 << 15)
#define C_INTR (1 << 10)
#define C_INTT (1 << 9)
#define C_INTD (1 << 8)
#define C_ST (1 << 7)
#define C_CLEAR (1 << 5)
#define C_READ (1 << 0)

#define S_CLKT (1 << 9)
#define S_ERR (1 << 8)
#define S_RXF (1 << 7)
#define S_TXE (1 << 6)
#define S_RXD (1 << 5)
#define S_TXD (1 << 4)
#define S_RXR (1 << 3)
#define S_TXW (1 << 2)
#define S_DONE (1 << 1)
#define S_TA (1 << 0)

#define I2C_TIMEOUT 10000  // Random value

typedef enum { I2C_100KHZ = 100000, I2C_400KHZ = 400000, I2C_1MHZ = 1000000 } I2CClockSpeed;

typedef enum { I2CS_SUCCESS, I2CS_ACK_ERROR, I2CS_DATA_LOSS, I2CS_CLOCK_TIMEOUT } I2CStatus;

typedef struct {
  reg32 control;
  reg32 status;
  reg32 data_length;
  reg32 slave_address;
  reg32 fifo;
  reg32 div;
  reg32 delay;
  reg32 clock_stretch;
} I2CRegs;

#define I2C_REGS ((I2CRegs *)(PBASE + 0x00804000))

void i2c_init();
I2CStatus i2c_recv(u8 address, u8 *buffer, u32 size);
I2CStatus i2c_send(u8 address, u8 *buffer, u32 size);
