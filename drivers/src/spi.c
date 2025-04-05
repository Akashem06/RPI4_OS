#include "spi.h"

#include "gpio.h"

static void spi_select_cs(u8 line_number) {
  SPI0_REGS->cs = (SPI0_REGS->cs & ~(0x3)) | (line_number & 0x3);
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

  SPI0_REGS->cs = 0;
  SPI0_REGS->cs |= CS_CLEAR_RX | CS_CLEAR_TX;

  SPI0_REGS->clock = SPI_CLOCK_DIVIDER;

  SPI0_REGS->cs = 0;  // Default settings CPOL CPHA = 0
}

void spi_send_recv(u8 chip_select, u8 *sbuffer, u8 *rbuffer, u32 write_size, u32 read_size) {
  spi_select_cs(chip_select);
  u32 read_count = 0;
  u32 write_count = 0;

  // Activate SPI
  SPI0_REGS->cs |= CS_TA;

  while (read_count < read_size || write_count < write_size) {
    while (write_count < write_size && SPI0_REGS->cs & CS_TXD) {
      if (sbuffer) {
        SPI0_REGS->fifo = *sbuffer++;
      } else {
        SPI0_REGS->fifo = 0;
      }

      write_count++;
    }

    while (read_count < read_size && SPI0_REGS->cs & CS_RXD) {
      u32 data = SPI0_REGS->fifo;

      if (rbuffer) {
        *rbuffer++ = data;
      }

      read_count++;
    }
  }

  // Wait for transfer to complete
  while (!(SPI0_REGS->cs & CS_DONE)) {
  }
  SPI0_REGS->cs &= ~CS_TA;
}

void spi_send(u8 chip_select, u8 *data, u32 size) {
  spi_select_cs(chip_select);

  // Activate SPI
  SPI0_REGS->cs |= CS_TA;

  for (u32 i = 0; i < size; i++) {
    while (!(SPI0_REGS->cs & CS_TXD)) {
    }
    SPI0_REGS->fifo = data[i];
    while (!(SPI0_REGS->cs & CS_DONE)) {
    }
  }

  SPI0_REGS->cs &= ~CS_TA;
}

void spi_recv(u8 chip_select, u8 *data, u32 size) {
  spi_select_cs(chip_select);

  // Activate SPI
  SPI0_REGS->cs |= CS_TA;

  for (u32 i = 0; i < size; i++) {
    while (!(SPI0_REGS->cs & CS_TXD)) {
    }
    SPI0_REGS->fifo = 0x00;

    while (!(SPI0_REGS->cs & CS_RXD)) {
    }
    data[i] = SPI0_REGS->fifo;
  }

  while (!(SPI0_REGS->cs & CS_DONE)) {
  }
  SPI0_REGS->cs &= ~CS_TA;
}

void spi_set_mode(u8 cpol, u8 cpha) {
  SPI0_REGS->cs =
      (SPI0_REGS->cs & ~(1 << CS_CPOL_SHIFT | 1 << CS_CPHA_SHIFT)) | (cpol << CS_CPOL_SHIFT) | (cpha << CS_CPHA_SHIFT);
}

void spi_set_clock_divider(u32 divider) {
  SPI0_REGS->clock = divider;
}
