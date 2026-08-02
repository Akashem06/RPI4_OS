#pragma once

/*******************************************************************************************************************************
 * @file   mini_uart.h
 *
 * @brief  Mini UART (AUX) header file for the BCM2711 SoC
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "aux_reg.h"
#include "base.h"
#include "common.h"
#include "error.h"

/* Intra-component Headers */

/**
 * @defgroup BCM2711_MiniUART BCM2711 Mini UART API
 * @brief    Mini UART (AUX) abstraction layer for the BCM2711 SoC from Broadcom
 * @{
 */

#define TX_PIN 14 /**< Mini UART TX GPIO pin */
#define RX_PIN 15 /**< Mini UART RX GPIO pin */

/**
 * @brief   Bring up the Mini UART at the datasheet baud rate
 */
void mini_uart_init();

/**
 * @brief   Receive a single byte, blocking until one arrives
 * @return  Received byte
 */
char mini_uart_receive();

/**
 * @brief   Transmit a single byte, blocking until the FIFO has room
 * @param   c Byte to send
 */
void mini_uart_transmit(char c);

/**
 * @brief   Transmit a null-terminated string, expanding \n to \r\n
 * @param   message String to send
 */
void mini_uart_transmit_string(char *message);

/**
 * @brief   Register the Mini UART as the "miniuart" char device
 * @return  SUCCESS or a negative ErrorCode
 */
ErrorCode mini_uart_register_device(void);

/** @} */
