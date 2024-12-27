#pragma once

/*******************************************************************************************************************************
 * @file   uart.h
 *
 * @brief  UART header file for the BCM2711 SoC
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */
#include "common.h"
#include "hardware.h"

/* Intra-component Headers */

/**
 * @defgroup BCM2711_UART BCM2711 UART API
 * @brief    UART Abstraction layer for the BCM2711 SoC from Broadcom
 * @{
 */

/**
 * @brief   UART Register definitions
 */
typedef struct {
  reg32 dr;      /**< Data Register */
  reg32 rsr_ecr; /**< Receive Status/Error Clear Register */
  reg32 reserved[4];
  reg32 fr; /**< Flag Register */
  reg32 reserved1;
  reg32 ilpr;  /**< IrDA Low-Power Counter Register */
  reg32 ibrd;  /**< Integer Baud Rate Register */
  reg32 fbrd;  /**< Fractional Baud Rate Register */
  reg32 lcrh;  /**< Line Control Register */
  reg32 cr;    /**< Control Register */
  reg32 ifls;  /**< Interrupt FIFO Level Select Register */
  reg32 imsc;  /**< Interrupt Mask Set/Clear Register */
  reg32 ris;   /**< Raw Interrupt Status Register */
  reg32 mis;   /**< Masked Interrupt Status Register */
  reg32 icr;   /**< Interrupt Clear Register */
  reg32 dmacr; /**< DMA Control Register */
} UartRegisters;

typedef struct {
  UartRegisters *uart;
  u32 baudrate;
  u8 tx;
  u8 rx;
  u8 cts;
  u8 rts;
  bool bluetooth;
} UartSettings;

#define UARTCLK 48000000  // 48 MHz
#define UART0_BASE (PBASE + 0x201000)
#define UART2_BASE (PBASE + 0x201400)
#define UART3_BASE (PBASE + 0x201600)
#define UART4_BASE (PBASE + 0x201800)
#define UART5_BASE (PBASE + 0x201A00)

#define UART0 ((UartRegisters *)(UART0_BASE))
#define UART2 ((UartRegisters *)(UART2_BASE))
#define UART3 ((UartRegisters *)(UART3_BASE))
#define UART4 ((UartRegisters *)(UART4_BASE))
#define UART5 ((UartRegisters *)(UART5_BASE))

#define UART_MAX_QUEUE (16 * 1024)

void uart_init(UartSettings *settings);
void uart_transmit(char c);
void uart_transmit_string(char *str);
char uart_receive();
void handle_uart0_irq();
bool uart_read_ready();

/** @} */
