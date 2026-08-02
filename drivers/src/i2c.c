/*******************************************************************************************************************************
 * @file   i2c.c
 *
 * @brief  I2C (BSC) driver for the BCM2711 SoC
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "bcm2711_periph_io.h"
#include "device.h"
#include "gpio.h"
#include "hardware.h"

/* Intra-component Headers */
#include "i2c.h"

/* Slave address used by the device read/write ops, set via ioctl */
static u8 i2c_target_address = 0;

static void i2c_setup(u8 address, u32 size, u32 control_flags) {
  REG_WR(I2C_REGS->slave_address, address);
  REG_WR(I2C_REGS->control, C_CLEAR);
  REG_WR(I2C_REGS->status, S_CLKT | S_ERR | S_DONE);
  REG_WR(I2C_REGS->data_length, size);
  REG_WR(I2C_REGS->control, C_I2CEN | C_ST | control_flags);
}

static ErrorCode i2c_check_status(u32 status, u32 count, u32 size) {
  if (status & S_ERR) {
    return ERR_DEVICE_NO_RESPONSE;
  } else if (status & S_CLKT) {
    return ERR_GEN_TIMEOUT;
  } else if (count < size) {
    return ERR_FS_IO;
  }
  return SUCCESS;
}

ErrorCode i2c_init(I2CClockSpeed clock_speed) {
  if (clock_speed > CORE_CLOCK_SPEED) {
    return ERR_GEN_INVALID_PARAM;
  }
  gpio_set_function(2, GF_ALT0);
  gpio_set_function(3, GF_ALT0);
  gpio_enable(2);
  gpio_enable(3);

  REG_WR(I2C_REGS->div, CORE_CLOCK_SPEED / clock_speed);
  return SUCCESS;
}

ErrorCode i2c_recv(u8 address, u8 *buffer, u32 size) {
  u32 count = 0U;
  int timeout = I2C_TIMEOUT;

  i2c_setup(address, size, C_READ);

  while (!(REG_RD(I2C_REGS->status) & S_DONE) && timeout > 0) {
    if (REG_RD(I2C_REGS->status) & (S_ERR | S_CLKT)) {
      break;
    }

    while (count < size && REG_RD(I2C_REGS->status) & S_RXD) {
      buffer[count] = REG_RD(I2C_REGS->fifo) & 0xFF;
      count++;
    }

    timeout--;
  }

  u32 status = REG_RD(I2C_REGS->status);
  REG_WR(I2C_REGS->status, S_DONE);

  return i2c_check_status(status, count, size);
}

ErrorCode i2c_send(u8 address, u8 *buffer, u32 size) {
  u32 count = 0U;
  int timeout = I2C_TIMEOUT;

  i2c_setup(address, size, 0);

  while (!(REG_RD(I2C_REGS->status) & S_DONE) && timeout > 0) {
    if (REG_RD(I2C_REGS->status) & (S_ERR | S_CLKT)) {
      break;
    }

    while (count < size && REG_RD(I2C_REGS->status) & S_TXD) {
      REG_WR(I2C_REGS->fifo, buffer[count]);
      count++;
    }

    timeout--;
  }

  u32 status = REG_RD(I2C_REGS->status);
  REG_WR(I2C_REGS->status, S_DONE);

  return i2c_check_status(status, count, size);
}

/* Device wrapper, read/write target the address set by I2C_IOCTL_SET_ADDRESS */

static long i2c_dev_read(struct Device *dev, void *buf, u64 len) {
  (void)dev;
  ErrorCode ret = i2c_recv(i2c_target_address, buf, (u32)len);
  return IS_ERROR(ret) ? ret : (long)len;
}

static long i2c_dev_write(struct Device *dev, const void *buf, u64 len) {
  (void)dev;
  ErrorCode ret = i2c_send(i2c_target_address, (u8 *)buf, (u32)len);
  return IS_ERROR(ret) ? ret : (long)len;
}

static ErrorCode i2c_dev_ioctl(struct Device *dev, u32 cmd, u64 arg) {
  (void)dev;
  switch (cmd) {
    case I2C_IOCTL_SET_ADDRESS:
      i2c_target_address = arg & 0x7F;
      return SUCCESS;
    default:
      return ERR_SYS_NOT_SUPPORTED;
  }
}

static const struct DeviceOps i2c_dev_ops = {
  .read = i2c_dev_read,
  .write = i2c_dev_write,
  .ioctl = i2c_dev_ioctl,
};

static struct Device i2c_dev = {
  .name = "i2c",
  .type = DEVICE_TYPE_CHAR,
  .ops = &i2c_dev_ops,
};

ErrorCode i2c_register_device(void) {
  return device_register(&i2c_dev);
}
