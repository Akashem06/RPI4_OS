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

/** @brief  Peripheral base address */
#define PERIPHERAL_BASE_ADDRESS 0xFE000000U

/** @brief  Core clock speed */
#define CORE_CLOCK_SPEED 1500000000U

#define PAGE_SHIFT 12
#define TABLE_SHIFT 9
#define SECTION_SHIFT (PAGE_SHIFT + TABLE_SHIFT)

/** @brief  Memory page size of 4096 Kbytes */
#define PAGE_SIZE (1U << PAGE_SHIFT)

#define SECTION_SIZE (1 << SECTION_SHIFT)

#define LOW_MEMORY (4 * 1024 * 1024)     // 4MB
#define HIGH_MEMORY (256 * 1024 * 1024)  // 256MB

#define PAGING_MEMORY (HIGH_MEMORY - LOW_MEMORY)
#define PAGING_PAGES (PAGING_MEMORY / PAGE_SIZE)

/** @} */
