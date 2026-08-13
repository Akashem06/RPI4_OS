#pragma once

/*******************************************************************************************************************************
 * @file   notify.h
 *
 * @brief  One-shot notification / completion API
 *
 * @date   2026-08-05
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

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
 * @brief   One-shot, sticky completion
 * @details Once signalled it stays signalled, so a wait that arrives after the signal returns
 *          immediately (no lost wakeup). Reused across events via notif_reset. This is the
 *          Bluetooth stack's command-complete primitive.
 */
typedef struct {
  volatile u32 notified; /**< Non-zero once signalled */
  struct Spinlock lock;  /**< Guards the flag and the test-and-block window */
} Notif;

/**
 * @brief   Initialize a notification to the un-signalled state
 * @param   notif Pointer to a notification struct
 */
void notif_init(Notif *notif);

/**
 * @brief   Block until the notification is signalled (returns at once if already signalled)
 * @param   notif Pointer to a notification struct
 */
void notif_wait(Notif *notif);

/**
 * @brief   Block until signalled or the timeout elapses
 * @param   notif      Pointer to a notification struct
 * @param   timeout_ms Milliseconds to wait before giving up
 * @return  SUCCESS if signalled, ERR_GEN_TIMEOUT on timeout
 */
ErrorCode notif_wait_timeout(Notif *notif, u32 timeout_ms);

/**
 * @brief   Signal the notification, waking all waiters
 * @param   notif Pointer to a notification struct
 */
void notif_signal(Notif *notif);

/**
 * @brief   Reset a notification so it can be waited on again
 * @param   notif Pointer to a notification struct
 */
void notif_reset(Notif *notif);

/** @} */
