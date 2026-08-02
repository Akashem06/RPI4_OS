/*******************************************************************************************************************************
 * @file   uart.c
 *
 * @brief  UART driver for the BCM2711 SoC
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
#include "uart.h"

UartSettings *config;

bool uart_read_ready() {
  return !(REG_RD(config->uart->fr) & (1 << 4));
}

void uart_transmit(char c) {
  while (REG_RD(config->uart->fr) & (1 << 5));  // Wait until TX FIFO is not full
  REG_WR(config->uart->dr, c);
}

char uart_receive() {
  while (REG_RD(config->uart->fr) & (1 << 4));  // Wait until RX FIFO is not empty
  return REG_RD(config->uart->dr) & 0xFF;
}

void uart_transmit_string(char *str) {
  while (*str) {
    if (*str == '\n') {
      uart_transmit('\r');
    }

    uart_transmit(*str);  // Send char at current address
    str++;                // Increment memory address, going to next char
  }
}

void uart_init(UartSettings *settings) {
  config = settings;
  if (config->bluetooth) {
    gpio_set_function(30, GF_ALT3);  // CTS0
    gpio_set_function(31, GF_ALT3);  // RTS0
    gpio_set_function(32, GF_ALT3);  // TXD0
    gpio_set_function(33, GF_ALT3);  // RXD0

    REG_WR(config->uart->cr, 0);

    // Clear all interrupts
    REG_WR(config->uart->icr, 0x7FF);

    // Set baud rate
    REG_WR(config->uart->ibrd, 26);
    REG_WR(config->uart->fbrd, 3);

    u32 ifls = REG_RD(config->uart->ifls) & ~0x3F;  // Clear all FIFO level bits
    ifls |= (2 << 3);                               // RX FIFO trigger at 1/2 full
    ifls |= (2 << 0);                               // TX FIFO trigger at 1/2 empty
    REG_WR(config->uart->ifls, ifls);

    // 8 bits, no parity, 1 stop bit
    REG_WR(config->uart->lcrh, (1 << 4) | (1 << 5) | (1 << 6));

    // Enable interrupts: RX, TX, and Overrun
    REG_WR(config->uart->imsc, (1 << 4) | (1 << 5) | (1 << 6) | (1 << 10));

    // Enable UART, TX, RX, CTS, and RTS
    REG_WR(config->uart->cr, (1 << 0) | (1 << 8) | (1 << 9) | (1 << 11) | (1 << 14) | (1 << 15));
  } else {
    if (config->uart == UART0) {
      // TODO: Add functionality for pins 36/37. Right now they initalized wrong
      gpio_set_function(config->tx, GF_ALT0);
      gpio_set_function(config->rx, GF_ALT0);
    } else if (config->uart == UART2 || config->uart == UART3 || config->uart == UART4 || config->uart == UART5) {
      gpio_set_function(config->tx, GF_ALT4);
      gpio_set_function(config->rx, GF_ALT4);
    }
    gpio_enable(config->tx);
    gpio_enable(config->rx);

    REG_WR(config->uart->cr, 0);  // Disable UART during setup

    // UART Reference Clock Freq = 48MHz
    // baud divisor = UARTCLK / (16 * baud_rate)
    //               = 48 * 10^6 / (16 * 115200) = 26.0416666667
    //  integer part = 26
    //  fractional part = (int) ((0.0416666667 * 64) + 0.5) = roughly 3
    // TODO: Add custom calculation. Will likely use an init struct + figure out FPU :(
    REG_WR(config->uart->ibrd, 26);
    REG_WR(config->uart->fbrd, 3);

    REG_WR(config->uart->lcrh, (1 << 4) | (1 << 5) | (1 << 6));  // 8 bits, no parity, 1 stop bit
    REG_WR(config->uart->imsc, (1 << 4));                        // Sets RX ISR
    REG_WR(config->uart->cr, (1 << 0) | (1 << 8) | (1 << 9));    // Enable UART, TX, and RX
  }
}

/* Device wrapper, exposes UART0 through the unified device interface */

static long uart_dev_read(struct Device *dev, void *buf, u64 len) {
  (void)dev;
  char *out = buf;
  for (u64 i = 0; i < len; i++) {
    out[i] = uart_receive();
  }
  return (long)len;
}

static long uart_dev_write(struct Device *dev, const void *buf, u64 len) {
  (void)dev;
  const char *in = buf;
  for (u64 i = 0; i < len; i++) {
    uart_transmit(in[i]);
  }
  return (long)len;
}

static const struct DeviceOps uart_dev_ops = {
  .read = uart_dev_read,
  .write = uart_dev_write,
};

static struct Device uart0_dev = {
  .name = "uart0",
  .type = DEVICE_TYPE_CHAR,
  .ops = &uart_dev_ops,
};

ErrorCode uart_register_device(void) {
  return device_register(&uart0_dev);
}
