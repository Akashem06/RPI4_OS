#pragma once

/*******************************************************************************************************************************
 * @file   semaphore.h
 *
 * @brief  Counting semaphore API
 *
 * @date   2026-08-05
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */
#include "common.h"
#include "error.h"

/* Intra-component Headers */
#include "spinlock.h"

/**
 * @defgroup ConcurrencyUtils Concurrency Utilities
 * @brief    Libraries to support Concurrency
 * @{
 */

/**
 * @brief   Counting semaphore storage
 */
typedef struct {
  volatile int count;  /**< Available permits, waiters block while <= 0 */
  struct Spinlock lock; /**< Guards count and the test-and-block window */
} Semaphore;

/**
 * @brief   Initialize a semaphore
 * @param   sem     Pointer to a semaphore struct
 * @param   initial Starting permit count
 */
void semaphore_init(Semaphore *sem, int initial);

/**
 * @brief   Take a permit, blocking until one is available
 * @param   sem Pointer to a semaphore struct
 */
void semaphore_wait(Semaphore *sem);

/**
 * @brief   Take a permit, blocking until one is available or the timeout elapses
 * @param   sem        Pointer to a semaphore struct
 * @param   timeout_ms Milliseconds to wait before giving up
 * @return  SUCCESS if a permit was taken, ERR_GEN_TIMEOUT on timeout
 */
ErrorCode semaphore_wait_timeout(Semaphore *sem, u32 timeout_ms);

/**
 * @brief   Take a permit only if one is immediately available
 * @param   sem Pointer to a semaphore struct
 * @return  true if a permit was taken, false otherwise
 */
bool semaphore_try_wait(Semaphore *sem);

/**
 * @brief   Release a permit, waking one waiter if any
 * @param   sem Pointer to a semaphore struct
 */
void semaphore_post(Semaphore *sem);

/** @} */
