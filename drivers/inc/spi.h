#pragma once

#include "base.h"
#include "common.h"

#define CS_LEN_LONG (1 << 25)
#define CS_DMA_LEN (1 << 24)
#define CS_CSPOL2 (1 << 23)
#define CS_CSPOL1 (1 << 22)
#define CS_CSPOL0 (1 << 21)
#define CS_RXF (1 << 20)
#define CS_RXR (1 << 19)
#define CS_TXD (1 << 18)
#define CS_RXD (1 << 17)
#define CS_DONE (1 << 16)
#define CS_LEN (1 << 13)
#define CS_REN (1 << 12)
#define CS_ADCS (1 << 11)
#define CS_INTR (1 << 10)
#define CS_INTD (1 << 9)
#define CS_DMAEN (1 << 8)
#define CS_TA (1 << 7)
#define CS_CSPOL (1 << 6)
#define CS_CLEAR_RX (1 << 5)
#define CS_CLEAR_TX (1 << 4)
#define CS_CPOL_SHIFT 3
#define CS_CPHA_SHIFT 2
#define CS_CS (1 << 0)
#define CS_CS_SHIFT 0

#define SPI_CLOCK_DIVIDER 128

typedef struct {
  reg32 cs;
  reg32 fifo;
  reg32 clock;
  reg32 data_length;
  reg32 ltoh;
  reg32 dc;
} Spi0Regs;

#define SPI0_REGS ((Spi0Regs *)(PBASE + 0x00204000))

void spi_init();
void spi_send_recv(u8 chip_select, u8 *sbuffer, u8 *rbuffer, u32 write_size, u32 read_size);
void spi_send(u8 chip_select, u8 *data, u32 size);
void spi_recv(u8 chip_select, u8 *data, u32 size);
void spi_set_mode(u8 cpol, u8 cpha);
void spi_set_clock_divider(u32 divider);
