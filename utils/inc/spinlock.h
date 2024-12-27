#pragma once

/*******************************************************************************************************************************
 * @file   spinlock.h
 *
 * @brief  Spinlock API
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
 * @defgroup ConcurrencyUtils Concurrency Utilities
 * @brief    Libraries to support Concurrency
 * @{
 */

/**
 * @brief   Spinlock storage
 */
typedef struct {
  volatile u64 lock; /**< Stores the current state of the spinlock */
} Spinlock;

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
      "   cbnz w2, 1b         \n"
      : "=&r"(tmp)
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

  asm volatile(
      "stlr    wzr, [%0]     \n" /* Ensures everything is completed before setting the lock to 0 */
      :
      : "r"(&lock->lock)
      : "memory");
}

/** @} */
