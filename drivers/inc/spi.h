#pragma once

/*******************************************************************************************************************************
 * @file   spi.h
 *
 * @brief  SPI0 header file for the BCM2711 SoC
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "base.h"
#include "common.h"
#include "error.h"

/* Intra-component Headers */

/**
 * @defgroup BCM2711_SPI BCM2711 SPI API
 * @brief    SPI0 abstraction layer for the BCM2711 SoC from Broadcom
 * @{
 */

#define CS_LEN_LONG (1 << 25) /**< Enable long data word */
#define CS_DMA_LEN (1 << 24)  /**< Enable DMA in LoSSI mode */
#define CS_CSPOL2 (1 << 23)   /**< Chip select 2 polarity */
#define CS_CSPOL1 (1 << 22)   /**< Chip select 1 polarity */
#define CS_CSPOL0 (1 << 21)   /**< Chip select 0 polarity */
#define CS_RXF (1 << 20)      /**< RX FIFO full */
#define CS_RXR (1 << 19)      /**< RX FIFO needs reading */
#define CS_TXD (1 << 18)      /**< TX FIFO has space */
#define CS_RXD (1 << 17)      /**< RX FIFO has data */
#define CS_DONE (1 << 16)     /**< Transfer done */
#define CS_LEN (1 << 13)      /**< LoSSI enable */
#define CS_REN (1 << 12)      /**< Read enable */
#define CS_ADCS (1 << 11)     /**< Auto deassert chip select */
#define CS_INTR (1 << 10)     /**< Interrupt on RXR */
#define CS_INTD (1 << 9)      /**< Interrupt on DONE */
#define CS_DMAEN (1 << 8)     /**< DMA enable */
#define CS_TA (1 << 7)        /**< Transfer active */
#define CS_CSPOL (1 << 6)     /**< Chip select polarity */
#define CS_CLEAR_RX (1 << 5)  /**< Clear RX FIFO */
#define CS_CLEAR_TX (1 << 4)  /**< Clear TX FIFO */
#define CS_CPOL_SHIFT 3       /**< Clock polarity bit */
#define CS_CPHA_SHIFT 2       /**< Clock phase bit */
#define CS_CS (1 << 0)        /**< Chip select field */
#define CS_CS_SHIFT 0         /**< Chip select field shift */

#define SPI_CLOCK_DIVIDER 128 /**< Default clock divider */

/**
 * @brief   ioctl commands for the "spi" device
 */
typedef enum {
  SPI_IOCTL_SELECT_CS,         /**< arg = chip select line for subsequent read/write */
  SPI_IOCTL_SET_MODE,          /**< arg = cpol | (cpha << 1) */
  SPI_IOCTL_SET_CLOCK_DIVIDER, /**< arg = clock divider */
} SpiIoctl;

/**
 * @brief   SPI0 Register definitions
 */
typedef struct {
  reg32 cs;          /**< Control/status */
  reg32 fifo;        /**< TX/RX FIFO */
  reg32 clock;       /**< Clock divider */
  reg32 data_length; /**< DMA data length */
  reg32 ltoh;        /**< LoSSI output hold delay */
  reg32 dc;          /**< DMA DREQ controls */
} Spi0Regs;

#define SPI0_REGS ((Spi0Regs *)(PBASE + 0x00204000)) /**< SPI0 register block */

/**
 * @brief   Bring up SPI0 on the default pins and clock
 */
void spi_init();

/**
 * @brief   Full-duplex transfer, clocking out sbuffer while reading into rbuffer
 * @param   chip_select Chip select line
 * @param   sbuffer     Bytes to send, or NULL to send zeros
 * @param   rbuffer     Buffer for received bytes, or NULL to discard
 * @param   write_size  Bytes to send
 * @param   read_size   Bytes to read
 */
void spi_send_recv(u8 chip_select, u8 *sbuffer, u8 *rbuffer, u32 write_size, u32 read_size);

/**
 * @brief   Send bytes on the given chip select
 * @param   chip_select Chip select line
 * @param   data        Bytes to send
 * @param   size        Bytes to send
 */
void spi_send(u8 chip_select, u8 *data, u32 size);

/**
 * @brief   Receive bytes on the given chip select
 * @param   chip_select Chip select line
 * @param   data        Destination buffer
 * @param   size        Bytes to read
 */
void spi_recv(u8 chip_select, u8 *data, u32 size);

/**
 * @brief   Set the clock polarity and phase
 * @param   cpol Clock polarity
 * @param   cpha Clock phase
 */
void spi_set_mode(u8 cpol, u8 cpha);

/**
 * @brief   Set the clock divider
 * @param   divider Clock divider
 */
void spi_set_clock_divider(u32 divider);

/**
 * @brief   Register SPI0 as the "spi" char device
 * @return  SUCCESS or a negative ErrorCode
 */
ErrorCode spi_register_device(void);

/** @} */
