#pragma once

/*******************************************************************************************************************************
 * @file   mutex.h
 *
 * @brief  Mutex API
 *
 * @date   2025-01-05
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

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
 * @brief   Mutex storage
 */
typedef struct {
  volatile u32 state; /* Lock state: 0 = unlocked, 1 = locked */
  // TaskId owner;           /* Thread ID of the owner */
  u32 lock_count;         /* For recursive mutex support (Locking when already locked) */
  u32 original_priority;  /* Original priority of the owner */
  struct thread *waiting; /* List of waiting threads */
} Mutex;

/**
 * @brief   Initialize a mutex
 * @param   mutex Pointer to a mutex struct
 */
void mutex_init(Mutex *mutex);

/**
 * @brief Attempt to acquire mutex
 * @return TRUE if successful, FALSE if already locked
 */
bool mutex_lock(Mutex *mutex);

/**
 * @brief Attempt to unlock mutex
 * @return TRUE if successful, FALSE if already locked
 */
bool mutex_unlock(Mutex *mutex);

/** @} */
