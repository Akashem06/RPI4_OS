#pragma once

/*******************************************************************************************************************************
 * @file   i2c.h
 *
 * @brief  I2C (BSC) header file for the BCM2711 SoC
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
 * @defgroup BCM2711_I2C BCM2711 I2C API
 * @brief    I2C (BSC) abstraction layer for the BCM2711 SoC from Broadcom
 * @{
 */

#define C_I2CEN (1 << 15) /**< I2C enable */
#define C_INTR (1 << 10)  /**< Interrupt on RX */
#define C_INTT (1 << 9)   /**< Interrupt on TX */
#define C_INTD (1 << 8)   /**< Interrupt on DONE */
#define C_ST (1 << 7)     /**< Start transfer */
#define C_CLEAR (1 << 5)  /**< Clear FIFO */
#define C_READ (1 << 0)   /**< Read transfer */

#define S_CLKT (1 << 9) /**< Clock stretch timeout */
#define S_ERR (1 << 8)  /**< ACK error */
#define S_RXF (1 << 7)  /**< RX FIFO full */
#define S_TXE (1 << 6)  /**< TX FIFO empty */
#define S_RXD (1 << 5)  /**< RX FIFO has data */
#define S_TXD (1 << 4)  /**< TX FIFO has space */
#define S_RXR (1 << 3)  /**< RX FIFO needs reading */
#define S_TXW (1 << 2)  /**< TX FIFO needs writing */
#define S_DONE (1 << 1) /**< Transfer done */
#define S_TA (1 << 0)   /**< Transfer active */

#define I2C_TIMEOUT 10000 /**< Poll iterations before giving up */

/**
 * @brief   Supported I2C bus clock speeds
 */
typedef enum {
  I2C_100KHZ = 100000,  /**< Standard mode */
  I2C_400KHZ = 400000,  /**< Fast mode */
  I2C_1MHZ = 1000000,   /**< Fast mode plus */
} I2CClockSpeed;

/**
 * @brief   ioctl commands for the "i2c" device
 */
typedef enum {
  I2C_IOCTL_SET_ADDRESS, /**< arg = 7-bit slave address for subsequent read/write */
} I2CIoctl;

/**
 * @brief   I2C Register definitions
 */
typedef struct {
  reg32 control;       /**< Control */
  reg32 status;        /**< Status */
  reg32 data_length;   /**< Data length */
  reg32 slave_address; /**< Slave address */
  reg32 fifo;          /**< Data FIFO */
  reg32 div;           /**< Clock divider */
  reg32 delay;         /**< Data delay */
  reg32 clock_stretch; /**< Clock stretch timeout */
} I2CRegs;

#define I2C_REGS ((I2CRegs *)(PBASE + 0x00804000)) /**< I2C1 register block */

/**
 * @brief   Bring up the I2C bus at the given clock speed
 * @param   clock_speed Target bus speed
 * @return  SUCCESS, or ERR_GEN_INVALID_PARAM if the speed is out of range
 */
ErrorCode i2c_init(I2CClockSpeed clock_speed);

/**
 * @brief   Receive bytes from a slave
 * @param   address 7-bit slave address
 * @param   buffer  Destination buffer
 * @param   size    Bytes to read
 * @return  SUCCESS or a negative ErrorCode
 */
ErrorCode i2c_recv(u8 address, u8 *buffer, u32 size);

/**
 * @brief   Send bytes to a slave
 * @param   address 7-bit slave address
 * @param   buffer  Source buffer
 * @param   size    Bytes to write
 * @return  SUCCESS or a negative ErrorCode
 */
ErrorCode i2c_send(u8 address, u8 *buffer, u32 size);

/**
 * @brief   Register the I2C bus as the "i2c" char device
 * @return  SUCCESS or a negative ErrorCode
 */
ErrorCode i2c_register_device(void);

/** @} */
