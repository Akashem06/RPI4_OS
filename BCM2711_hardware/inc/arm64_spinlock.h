#pragma once

/*******************************************************************************************************************************
 * @file   arm64_spinlock.h
 *
 * @brief  ARM64 Spinlock API
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "arm64_barrier.h"
#include "common.h"

/* Intra-component Headers */

/**
 * @defgroup BCM2711_Hardware BCM2711 Hardware layer
 * @brief    Abstraction layer for the BCM2711 SoC from Broadcom
 * @{
 */

/* Included via spinlock.h, primitives are static inline so each TU gets its own copy */

/**
 * @brief   Lock the spinlock
 * @param   lock Pointer to a spinlock struct
 */
static inline void spin_lock(struct Spinlock *lock) {
  u64 temp;

  asm volatile(
      "1: ldaxr %w0, [%1]     \n"
      "   cbnz %w0, 1b        \n" /* Backwards branch to local branch '1' if the lock is not 0 */
      "   mov %w0, #1         \n"
      "   stxr w2, %w0, [%1]  \n" /* Store the lock into the struct */
      "   cbnz w2, 1b         \n" /* Retry if the store fails */
      : "=&r"(temp)
      : "r"(&lock->lock)
      : "memory", "w2");

  dmb();
}

/**
 * @brief   Unlock the spinlock
 * @param   lock Pointer to a spinlock struct
 */
static inline void spin_unlock(struct Spinlock *lock) {
  dmb();

  asm volatile("stlr    wzr, [%0]     \n" /* Ensures everything is completed before setting the lock to 0 */
               :
               : "r"(&lock->lock)
               : "memory");
}

/**
 * @brief   Save DAIF, mask IRQs, then take the lock
 * @param   lock Pointer to a spinlock struct
 * @return  Saved DAIF to pass to spin_unlock_irqrestore
 */
static inline u64 spin_lock_irqsave(struct Spinlock *lock) {
  u64 flags;

  asm volatile("mrs %0, daif    \n" : "=r"(flags));
  asm volatile("msr daifset, #2 \n" ::: "memory"); /* Mask IRQs */

  spin_lock(lock);

  return flags;
}

/**
 * @brief   Release the lock then restore DAIF
 * @param   lock  Pointer to a spinlock struct
 * @param   flags DAIF returned by spin_lock_irqsave
 */
static inline void spin_unlock_irqrestore(struct Spinlock *lock, u64 flags) {
  spin_unlock(lock);

  asm volatile("msr daif, %0    \n" ::"r"(flags) : "memory");
}