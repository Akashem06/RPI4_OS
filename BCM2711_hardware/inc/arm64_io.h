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
#include "bcm2711.h"

/**
 * @defgroup BCM2711_Hardware BCM2711 Hardware layer
 * @brief    Abstraction layer for the BCM2711 SoC from Broadcom
 * @{
 */

/** @brief  Custom definition for IO memory */
#define __io_memory __attribute__((noderef))

/**
 * @brief   Write 1 byte of data to a provided address
 * @param   address Physical memory address
 * @param   value 1 byte value to be written
 */
static inline void write8(u64 address, u8 value) {
  asm volatile("strb %w0, [%1]         \n" /* Store the byte of data */
               :                           /* no outputs */
               : "r"(value), "r"(address)
               : "memory");
}

/**
 * @brief   Write 2 bytes of data to a provided address
 * @param   address Physical memory address
 * @param   value 2 byte value to be written
 */
static inline void write16(u64 address, u16 value) {
  asm volatile("strh %w0, [%1]         \n" /* Store 2 bytes (half) of data */
               :                           /* no outputs */
               : "r"(value), "r"(address)
               : "memory");
}

/**
 * @brief   Write 4 bytes of data to a provided address
 * @param   address Physical memory address
 * @param   value 4 byte value to be written
 */
static inline void write32(u64 address, u32 value) {
  asm volatile("str %w0, [%1]         \n" /* Store 4 bytes of data */
               :                          /* no outputs */
               : "r"(value), "r"(address)
               : "memory");
}

/**
 * @brief   Write 8 bytes of data to a provided address
 * @param   address Physical memory address
 * @param   value 8 byte value to be written
 */
static inline void write64(u64 address, u64 value) {
  asm volatile("strd %w0, [%1]         \n" /* Store 8 bytes (double) of data */
               :                           /* no outputs */
               : "r"(value), "r"(address)
               : "memory");
}

/**
 * @brief   Read 1 byte of data from a provided address
 * @param   address Physical memory address
 */
u8 read8(u64 address) {
  u8 value;

  asm volatile("ldrb %w0, [%1]         \n" /* Read a byte of data */
               : "=r"(value)
               : "r"(address)
               : "memory");

  return value;
}

/**
 * @brief   Read 2 bytes of data from a provided address
 * @param   address Physical memory address
 */
u16 read16(u64 address) {
  u16 value;

  asm volatile("ldrh %w0, [%1]         \n" /* Read 2 bytes (half) of data */
               : "=r"(value)
               : "r"(address)
               : "memory");

  return value;
}

/**
 * @brief   Read 4 bytes of data from a provided address
 * @param   address Physical memory address
 */
u32 read32(u64 address) {
  u32 value;

  asm volatile("ldr %w0, [%1]          \n" /* Read 4 bytes of data */
               : "=r"(value)
               : "r"(address)
               : "memory");

  return value;
}

/**
 * @brief   Read 8 bytes of data from a provided address
 * @param   address Physical memory address
 */
u64 read64(u64 address) {
  u64 value;

  asm volatile("ldrd %w0, [%1]         \n" /* Read 8 bytes (double) of data */
               : "=r"(value)
               : "r"(address)
               : "memory");

  return value;
}

/* Simple memory mapping function (basic version) */
static inline void __io_memory *ioremap(u64 physical_address, u64 size) {
  return (void __io_memory *)(physical_address + PERIPHERAL_BASE_ADDRESS);
}

/** @} */
