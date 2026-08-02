/*******************************************************************************************************************************
 * @file   mini_uart.c
 *
 * @brief  Mini UART (AUX) driver for the BCM2711 SoC
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
#include "mini_uart.h"

void mini_uart_transmit(char c) {
  while (!(REG_RD(AUX_REGS->mu_lsr) & 0x20));  // Checks if transmitter empty

  REG_WR(AUX_REGS->mu_io, c);
}

void mini_uart_transmit_string(char *str) {
  while (*str) {
    if (*str == '\n') {
      mini_uart_transmit('\r');
    }

    mini_uart_transmit(*str);  // Send char at current address
    str++;                     // Increment memory address, going to next char
  }
}

char mini_uart_receive() {
  while (!(REG_RD(AUX_REGS->mu_lsr) & 0x1));  // Checks if data ready

  return REG_RD(AUX_REGS->mu_io) & 0xFF;
}

void mini_uart_init() {
  gpio_set_function(TX_PIN, GF_ALT5);
  gpio_set_function(RX_PIN, GF_ALT5);

  gpio_enable(TX_PIN);
  gpio_enable(RX_PIN);

  REG_WR(AUX_REGS->irq_enables, 1);  // Enables ISR for all peripherals
  REG_WR(AUX_REGS->mu_control, 0);   // Disables UART transmitter and receiver
  REG_WR(AUX_REGS->mu_ier, 0xD);     // Enables ISR's for Mini UART
  REG_WR(AUX_REGS->mu_lcr, 3);       // Sets UART data format to 8bit messages
  REG_WR(AUX_REGS->mu_mcr, 0);       // Disables features like RTS and CTS

#if RPI_VERSION == 3
  // Defined in datasheet: System_Clock_Freq / ( 8 * (1 + mu_baudrate ) ) @ 250 MHz
  REG_WR(AUX_REGS->mu_baudrate, 270);
#elif RPI_VERSION == 4
  // Defined in datasheet: System_Clock_Freq / ( 8 * (1 + mu_baudrate ) ) @ 500 MHz
  REG_WR(AUX_REGS->mu_baudrate, 541);
#endif

  REG_WR(AUX_REGS->mu_control, 3);  // Enables transmitter and receiver

  mini_uart_transmit('\r');
  mini_uart_transmit('\r');
  mini_uart_transmit('\n');
}

/* Device wrapper, exposes the Mini UART through the unified device interface */

static long mini_uart_dev_read(struct Device *dev, void *buf, u64 len) {
  (void)dev;
  char *out = buf;
  for (u64 i = 0; i < len; i++) {
    out[i] = mini_uart_receive();
  }
  return (long)len;
}

static long mini_uart_dev_write(struct Device *dev, const void *buf, u64 len) {
  (void)dev;
  const char *in = buf;
  for (u64 i = 0; i < len; i++) {
    mini_uart_transmit(in[i]);
  }
  return (long)len;
}

static const struct DeviceOps mini_uart_dev_ops = {
  .read = mini_uart_dev_read,
  .write = mini_uart_dev_write,
};

static struct Device mini_uart_dev = {
  .name = "miniuart",
  .type = DEVICE_TYPE_CHAR,
  .ops = &mini_uart_dev_ops,
};

ErrorCode mini_uart_register_device(void) {
  return device_register(&mini_uart_dev);
}
