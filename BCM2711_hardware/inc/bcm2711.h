#pragma once

/*******************************************************************************************************************************
 * @file   bcm2711.h
 *
 * @brief  Main header file for the BCM2711 SoC
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup BCM2711_Hardware BCM2711 Hardware layer
 * @brief    Abstraction layer for the BCM2711 SoC from Broadcom
 * @{
 */

/** @brief  Peripheral base address, single source of truth for the SoC MMIO window */
#if RPI_VERSION == 3
#define PERIPHERAL_BASE_ADDRESS 0x3F000000U
#elif RPI_VERSION == 4
#define PERIPHERAL_BASE_ADDRESS 0xFE000000U /* Low peripheral mode */
#else
#error "NO RPI_VERSION DEFINED"
#endif

/** @brief  Core clock speed */
#define CORE_CLOCK_SPEED 1500000000U

#define PAGE_SHIFT 12
#define TABLE_SHIFT 9
#define SECTION_SHIFT (PAGE_SHIFT + TABLE_SHIFT)

/** @brief  Memory page size of 4096 Kbytes */
#define PAGE_SIZE (1U << PAGE_SHIFT)

#define SECTION_SIZE (1 << SECTION_SHIFT)

/** @} */
