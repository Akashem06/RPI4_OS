#pragma once

/*******************************************************************************************************************************
 * @file   bcm2711_periph_io.h
 *
 * @brief  BCM2711 input/output writing and reading abstraction
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "common.h"
#include "spinlock.h"

/* Intra-component Headers */
#include "arm64_io.h"
#include "bcm2711.h"

/**
 * @defgroup BCM2711_Hardware BCM2711 Hardware layer
 * @brief    Abstraction layer for the BCM2711 SoC from Broadcom
 * @{
 */

static inline void mmio_write(__io_memory void *address, u32 value) {
  dmb();
  /* Barrier before accessing peripheral memory as per datasheet */

  write32((u64)address, value);

  /* Barrier after accessing peripheral memory as per datasheet */
  dmb();
}

static inline u32 mmio_read(__io_memory void *address) {
  u32 value;

  dmb();
  /* Barrier before accessing peripheral memory as per datasheet */

  value = read32((u64)address);

  /* Barrier after accessing peripheral memory as per datasheet */
  dmb();

  return value;
}

/** @} */
