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

/* spin_lock() / spin_unlock() come from the arch header below */
#ifdef ARCH_ARM64
#include "arm64_spinlock.h"
#elif defined(ARCH_X86)
#include "x86_spinlock.h"
#else
#error "Unsupported architecture: define ARCH_ARM64 or ARCH_X86"
#endif

/** @} */
