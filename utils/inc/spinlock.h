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
struct Spinlock {
  volatile u64 lock; /**< Stores the current state of the spinlock */
};

#define SPIN_LOCK_INIT {0U}

/**
 * @brief   Lock the spinlock (architecture-specific implementation)
 * @param   lock Pointer to a spinlock struct
 */
void spin_lock(struct Spinlock *lock);

/**
 * @brief   Unlock the spinlock (architecture-specific implementation)
 * @param   lock Pointer to a spinlock struct
 */
void spin_unlock(struct Spinlock *lock);

/** @} */
