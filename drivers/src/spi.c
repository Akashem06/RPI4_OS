/*******************************************************************************************************************************
 * @file   spi.c
 *
 * @brief  SPI0 driver for the BCM2711 SoC
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "bcm2711_periph_io.h"
#include "device.h"
#include "gpio.h"

/* Intra-component Headers */
#include "spi.h"

/* Chip select used by the device read/write ops, set via ioctl */
static u8 spi_selected_cs = 0;

static void spi_select_cs(u8 line_number) {
  REG_WR(SPI0_REGS->cs, (REG_RD(SPI0_REGS->cs) & ~(0x3)) | (line_number & 0x3));
}

void spi_init() {
  gpio_set_function(7, GF_ALT0);
  gpio_set_function(8, GF_ALT0);
  gpio_set_function(9, GF_ALT0);
  gpio_set_function(10, GF_ALT0);
  gpio_set_function(11, GF_ALT0);
  gpio_enable(7);
  gpio_enable(8);
  gpio_enable(9);
  gpio_enable(10);
  gpio_enable(11);

  REG_WR(SPI0_REGS->cs, 0);
  REG_WR(SPI0_REGS->cs, REG_RD(SPI0_REGS->cs) | CS_CLEAR_RX | CS_CLEAR_TX);

  REG_WR(SPI0_REGS->clock, SPI_CLOCK_DIVIDER);

  REG_WR(SPI0_REGS->cs, 0);  // Default settings CPOL CPHA = 0
}

void spi_send_recv(u8 chip_select, u8 *sbuffer, u8 *rbuffer, u32 write_size, u32 read_size) {
  spi_select_cs(chip_select);
  u32 read_count = 0;
  u32 write_count = 0;

  // Activate SPI
  REG_WR(SPI0_REGS->cs, REG_RD(SPI0_REGS->cs) | CS_TA);

  while (read_count < read_size || write_count < write_size) {
    while (write_count < write_size && REG_RD(SPI0_REGS->cs) & CS_TXD) {
      REG_WR(SPI0_REGS->fifo, sbuffer ? sbuffer[write_count] : 0);
      write_count++;
    }

    while (read_count < read_size && REG_RD(SPI0_REGS->cs) & CS_RXD) {
      u32 data = REG_RD(SPI0_REGS->fifo);
      if (rbuffer) {
        rbuffer[read_count] = data;
      }
      read_count++;
    }
  }

  // Wait for transfer to complete
  while (!(REG_RD(SPI0_REGS->cs) & CS_DONE)) {
  }
  REG_WR(SPI0_REGS->cs, REG_RD(SPI0_REGS->cs) & ~CS_TA);
}

void spi_send(u8 chip_select, u8 *data, u32 size) {
  spi_select_cs(chip_select);

  // Activate SPI
  REG_WR(SPI0_REGS->cs, REG_RD(SPI0_REGS->cs) | CS_TA);

  for (u32 i = 0; i < size; i++) {
    while (!(REG_RD(SPI0_REGS->cs) & CS_TXD)) {
    }
    REG_WR(SPI0_REGS->fifo, data[i]);
    while (!(REG_RD(SPI0_REGS->cs) & CS_DONE)) {
    }
  }

  REG_WR(SPI0_REGS->cs, REG_RD(SPI0_REGS->cs) & ~CS_TA);
}

void spi_recv(u8 chip_select, u8 *data, u32 size) {
  spi_select_cs(chip_select);

  // Activate SPI
  REG_WR(SPI0_REGS->cs, REG_RD(SPI0_REGS->cs) | CS_TA);

  for (u32 i = 0; i < size; i++) {
    while (!(REG_RD(SPI0_REGS->cs) & CS_TXD)) {
    }
    REG_WR(SPI0_REGS->fifo, 0x00);

    while (!(REG_RD(SPI0_REGS->cs) & CS_RXD)) {
    }
    data[i] = REG_RD(SPI0_REGS->fifo);
  }

  while (!(REG_RD(SPI0_REGS->cs) & CS_DONE)) {
  }
  REG_WR(SPI0_REGS->cs, REG_RD(SPI0_REGS->cs) & ~CS_TA);
}

void spi_set_mode(u8 cpol, u8 cpha) {
  REG_WR(SPI0_REGS->cs, (REG_RD(SPI0_REGS->cs) & ~(1 << CS_CPOL_SHIFT | 1 << CS_CPHA_SHIFT)) |
                            (cpol << CS_CPOL_SHIFT) | (cpha << CS_CPHA_SHIFT));
}

void spi_set_clock_divider(u32 divider) {
  REG_WR(SPI0_REGS->clock, divider);
}

/* Device wrapper, read/write target the chip select set by SPI_IOCTL_SELECT_CS */

static long spi_dev_read(struct Device *dev, void *buf, u64 len) {
  (void)dev;
  spi_recv(spi_selected_cs, buf, (u32)len);
  return (long)len;
}

static long spi_dev_write(struct Device *dev, const void *buf, u64 len) {
  (void)dev;
  spi_send(spi_selected_cs, (u8 *)buf, (u32)len);
  return (long)len;
}

static ErrorCode spi_dev_ioctl(struct Device *dev, u32 cmd, u64 arg) {
  (void)dev;
  switch (cmd) {
    case SPI_IOCTL_SELECT_CS:
      spi_selected_cs = arg & 0x3;
      return SUCCESS;
    case SPI_IOCTL_SET_MODE:
      spi_set_mode(arg & 0x1, (arg >> 1) & 0x1);
      return SUCCESS;
    case SPI_IOCTL_SET_CLOCK_DIVIDER:
      spi_set_clock_divider((u32)arg);
      return SUCCESS;
    default:
      return ERR_SYS_NOT_SUPPORTED;
  }
}

static const struct DeviceOps spi_dev_ops = {
  .read = spi_dev_read,
  .write = spi_dev_write,
  .ioctl = spi_dev_ioctl,
};

static struct Device spi_dev = {
  .name = "spi",
  .type = DEVICE_TYPE_CHAR,
  .ops = &spi_dev_ops,
};

ErrorCode spi_register_device(void) {
  return device_register(&spi_dev);
}
