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
#include "spinlock.h"

/**
 * @defgroup ConcurrencyUtils Concurrency Utilities
 * @brief    Libraries to support Concurrency
 * @{
 */

struct TaskBlock; /* forward decl, the owning task, from scheduler.h */

/**
 * @brief   Mutex storage
 * @details A blocking, recursive, owner-tracked mutex. Contenders sleep on the mutex via the
 *          scheduler wait-channel core rather than spinning. Priority inheritance is a
 *          deferred follow-up (see utils/Docs).
 */
typedef struct {
  volatile u32 state;       /**< 0 = unlocked, 1 = locked */
  struct TaskBlock *owner;  /**< Task currently holding the mutex */
  u32 lock_count;           /**< Recursion depth for the owner */
  struct Spinlock lock;     /**< Guards the fields above and the test-and-block window */
} Mutex;

/**
 * @brief   Initialize a mutex to the unlocked state
 * @param   mutex Pointer to a mutex struct
 */
void mutex_init(Mutex *mutex);

/**
 * @brief   Acquire the mutex, blocking until it is free
 * @details Recursive for the current owner, each lock needs a matching unlock.
 * @param   mutex Pointer to a mutex struct
 * @return  true once held
 */
bool mutex_lock(Mutex *mutex);

/**
 * @brief   Acquire the mutex only if it is immediately available
 * @param   mutex Pointer to a mutex struct
 * @return  true if acquired, false if another task holds it
 */
bool mutex_trylock(Mutex *mutex);

/**
 * @brief   Release the mutex, waking a waiter when fully unlocked
 * @param   mutex Pointer to a mutex struct
 * @return  true on success, false if the caller is not the owner
 */
bool mutex_unlock(Mutex *mutex);

/** @} */
