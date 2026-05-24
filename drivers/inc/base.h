#pragma once

/*******************************************************************************************************************************
 * @file   base.h
 *
 * @brief  Peripheral base address alias for the driver layer
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "bcm2711.h"

/* Intra-component Headers */

/**
 * @defgroup BCM2711_Drivers BCM2711 Drivers
 * @brief    Driver layer for the BCM2711 SoC
 * @{
 */

/** @brief  PBASE aliases the HAL peripheral base, kept for existing driver headers */
#define PBASE PERIPHERAL_BASE_ADDRESS

/** @} */
