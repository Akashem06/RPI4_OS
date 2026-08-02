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
#include "error.h"
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
  reg32 dr;          /**< Data Register */
  reg32 rsr_ecr;     /**< Receive Status/Error Clear Register */
  reg32 reserved[4]; /**< Reserved */
  reg32 fr;          /**< Flag Register */
  reg32 reserved1;   /**< Reserved */
  reg32 ilpr;        /**< IrDA Low-Power Counter Register */
  reg32 ibrd;        /**< Integer Baud Rate Register */
  reg32 fbrd;        /**< Fractional Baud Rate Register */
  reg32 lcrh;        /**< Line Control Register */
  reg32 cr;          /**< Control Register */
  reg32 ifls;        /**< Interrupt FIFO Level Select Register */
  reg32 imsc;        /**< Interrupt Mask Set/Clear Register */
  reg32 ris;         /**< Raw Interrupt Status Register */
  reg32 mis;         /**< Masked Interrupt Status Register */
  reg32 icr;         /**< Interrupt Clear Register */
  reg32 dmacr;       /**< DMA Control Register */
} UartRegisters;

/**
 * @brief   UART instance configuration
 */
typedef struct {
  UartRegisters *uart; /**< Register block for this UART */
  u32 baudrate;        /**< Target baud rate */
  u8 tx;               /**< TX GPIO pin */
  u8 rx;               /**< RX GPIO pin */
  u8 cts;              /**< CTS GPIO pin */
  u8 rts;              /**< RTS GPIO pin */
  bool bluetooth;      /**< True to wire up the Bluetooth flow control pins */
} UartSettings;

#define UARTCLK 48000000 /**< UART reference clock, 48 MHz */

#define UART0_BASE (PBASE + 0x201000) /**< PL011 UART0 register base */
#define UART2_BASE (PBASE + 0x201400) /**< PL011 UART2 register base */
#define UART3_BASE (PBASE + 0x201600) /**< PL011 UART3 register base */
#define UART4_BASE (PBASE + 0x201800) /**< PL011 UART4 register base */
#define UART5_BASE (PBASE + 0x201A00) /**< PL011 UART5 register base */

#define UART0 ((UartRegisters *)(UART0_BASE)) /**< UART0 register block */
#define UART2 ((UartRegisters *)(UART2_BASE)) /**< UART2 register block */
#define UART3 ((UartRegisters *)(UART3_BASE)) /**< UART3 register block */
#define UART4 ((UartRegisters *)(UART4_BASE)) /**< UART4 register block */
#define UART5 ((UartRegisters *)(UART5_BASE)) /**< UART5 register block */

#define UART_MAX_QUEUE (16 * 1024) /**< Max queued bytes */

/**
 * @brief   Bring up a UART from the given settings
 * @param   settings UART instance configuration
 */
void uart_init(UartSettings *settings);

/**
 * @brief   Transmit a single byte, blocking until the TX FIFO has room
 * @param   c Byte to send
 */
void uart_transmit(char c);

/**
 * @brief   Transmit a null-terminated string, expanding \n to \r\n
 * @param   str String to send
 */
void uart_transmit_string(char *str);

/**
 * @brief   Receive a single byte, blocking until one arrives
 * @return  Received byte
 */
char uart_receive();

/**
 * @brief   UART0 receive interrupt handler
 */
void handle_uart0_irq();

/**
 * @brief   Check whether a byte is waiting in the RX FIFO
 * @return  True if a byte can be read without blocking
 */
bool uart_read_ready();

/**
 * @brief   Register the initialized UART0 as the "uart0" char device
 * @return  SUCCESS or a negative ErrorCode
 */
ErrorCode uart_register_device(void);

/** @} */
