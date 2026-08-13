#pragma once

/*******************************************************************************************************************************
 * @file   bcm2711_bus.h
 *
 * @brief  BCM2711 VideoCore bus-address translation for peripheral (DMA/GPU) access
 *
 * @date   2026-08-02
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

#define GPU_CACHED_BASE 0x40000000U    /**< Cached alias, faster GPU/DMA access */
#define GPU_UNCACHED_BASE 0xC0000000U  /**< Uncached alias, used for peripherals */
#define GPU_MEM_BASE GPU_UNCACHED_BASE /**< Peripherals talk over the uncached alias */

/** @brief Translate an ARM physical address into a VideoCore bus address */
#define BUS_ADDRESS(addr) (((addr) & ~0xC0000000U) | GPU_MEM_BASE)

/** @} */
