#pragma once

/*******************************************************************************************************************************
 * @file   arm64_barrier.h
 *
 * @brief  ARM64 instruction and memory barrier definitions
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

/**
 * @brief   Data memory barrier. Ensures memory access ordering is correct
 * @details Typically used when the order of memory accesses (reads and writes) is important to
 *          avoid pipelining hazards For example, in a multi-core system, a DMB instruction can
 *          ensure that all previous memory accesses are completed before new memory accesses are
 *          initiated, preventing issues with out-of-order execution
 */
static inline void dmb(void) {
  asm volatile("dmb sy" ::: "memory");
}

/**
 * @brief   Data synchronization barrier. Ensures that all previous memory accesses are completed
 *          before continuing with the next instruction
 * @details Typically used to enforce synchronization in multi-core or multi-threaded systems
 *          For example, in a multi-core system, DSB ensures that stores are propagated to other
 *          cores before further memory accesses occur
 */
static inline void dsb(void) {
  asm volatile("dsb sy" ::: "memory");
}

/**
 * @brief   Instruction synchronization barrier. Ensures all instructions in the pipeline are
 *          completed before moving on
 * @details Typically used when it is necessary to ensure that all previous instructions have
 *          completed before executing further instructions For example, after a context switch, ISB
 *          ensures that any speculatively executed instructions (like branch predictions) are
 *          discarded
 */
static inline void isb(void) {
  asm volatile("isb sy" ::: "memory");
}

/** @} */
