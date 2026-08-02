#pragma once

/*******************************************************************************************************************************
 * @file   gpio.h
 *
 * @brief  GPIO header file for the BCM2711 SoC
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "base.h"
#include "common.h"
#include "error.h"
#include "utils.h"

/* Intra-component Headers */

/**
 * @defgroup BCM2711_GPIO BCM2711 GPIO API
 * @brief    GPIO Abstraction layer for the BCM2711 SoC from Broadcom
 * @{
 */

/**
 * @brief   GPIO pin function select values
 */
typedef enum {
  GF_INPUT,      /**< Input */
  GF_OUTPUT,     /**< Output */
  GF_ALT0 = 4,   /**< Alternate function 0 */
  GF_ALT1,       /**< Alternate function 1 */
  GF_ALT2,       /**< Alternate function 2 */
  GF_ALT3,       /**< Alternate function 3 */
  GF_ALT4 = 3,   /**< Alternate function 4 */
  GF_ALT5 = 2,   /**< Alternate function 5 */
} GpioFunctions;

/**
 * @brief   ioctl commands for the "gpio" device, arg carries the pin
 */
typedef enum {
  GPIO_IOCTL_SET_FUNCTION, /**< arg = pin | (GpioFunctions << 8) */
  GPIO_IOCTL_ENABLE,       /**< arg = pin */
  GPIO_IOCTL_SET_HIGH,     /**< arg = pin */
  GPIO_IOCTL_SET_LOW,      /**< arg = pin */
} GpioIoctl;

/**
 * @brief   A pair of banked 32-bit GPIO registers
 */
typedef struct {
  reg32 reserved; /**< Reserved */
  reg32 data[2];  /**< Bank 0 and bank 1 */
} GpioPinData;

/**
 * @brief   GPIO Register definitions
 * @details https://datasheets.raspberrypi.com/bcm2711/bcm2711-peripherals.pdf
 */
typedef struct {
  reg32 function_select[6];      /**< Function select */
  GpioPinData output_set;        /**< Output set */
  GpioPinData output_clear;      /**< Output clear */
  GpioPinData level;             /**< Pin level */
  GpioPinData event_detect;      /**< Event detect status */
  GpioPinData rising_edge;       /**< Rising edge detect enable */
  GpioPinData falling_edge;      /**< Falling edge detect enable */
  GpioPinData pin_high;          /**< High detect enable */
  GpioPinData pin_low;           /**< Low detect enable */
  GpioPinData async_rising_edge; /**< Async rising edge detect enable */
  GpioPinData async_falling_edge;/**< Async falling edge detect enable */
  reg32 reserved;                /**< Reserved */
  reg32 pupd_enable;             /**< Pull up/down enable */
  reg32 pupd_enable_clocks[2];   /**< Pull up/down enable clocks */
} GpioRegisters;

#define GPIO_BASE (PBASE + 0x00200000)                     /**< GPIO register base address */
#define GPIO_REGS ((volatile GpioRegisters *)(GPIO_BASE))  /**< GPIO register block */

/**
 * @brief   Select the function of a GPIO pin
 * @param   pin_number Pin to configure
 * @param   func Function to select
 */
void gpio_set_function(u8 pin_number, GpioFunctions func);

/**
 * @brief   Clear the pull up/down state of a GPIO pin
 * @param   pin_number Pin to configure
 */
void gpio_enable(u8 pin_number);

/**
 * @brief   Drive a GPIO pin high
 * @param   pin_number Pin to set
 */
void gpio_set_high(u8 pin_number);

/**
 * @brief   Drive a GPIO pin low
 * @param   pin_number Pin to clear
 */
void gpio_set_low(u8 pin_number);

/**
 * @brief   Register the GPIO block as the "gpio" misc device
 * @return  SUCCESS or a negative ErrorCode
 */
ErrorCode gpio_register_device(void);

/** @} */
