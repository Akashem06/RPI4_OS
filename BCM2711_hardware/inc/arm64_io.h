#pragma once

/*******************************************************************************************************************************
 * @file   arm64_io.h
 *
 * @brief  ARM64 input/output writing and reading abstraction
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "common.h"

/* Intra-component Headers */

/**
 * @defgroup BCM2711_Hardware BCM2711 Hardware layer
 * @brief    Abstraction layer for the BCM2711 SOC from Broadcom
 * @{
 */

void write8(u64 address, u8 value);
void write16(u64 address, u16 value);
void write32(u64 address, u32 value);
void write64(u64 address, u64 value);

u8  read8(u64 address);
u16 read16(u64 address);
u32 read32(u64 address);
u64 read64(u64 address);

/** @} */
