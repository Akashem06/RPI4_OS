/*******************************************************************************************************************************
 * @file   notify.c
 *
 * @brief  One-shot completion built on the scheduler wait-channel core
 *
 * @date   2026-08-05
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "scheduler.h"
#include "timer.h"

/* Intra-component Headers */
#include "notify.h"

/* The notification address is its own wait channel. The flag is set under the same IRQ-masked
   lock the waiter tests it under, and it is sticky, so a signal that races an about-to-block
   waiter is seen on the re-test rather than lost. */

void notif_init(Notif *notif) {
  notif->notified = 0;
  notif->lock.lock = 0;
}

void notif_wait(Notif *notif) {
  u64 flags = spin_lock_irqsave(&notif->lock);

  while (!notif->notified) {
    spin_unlock(&notif->lock); /* keep IRQs masked across the block */
    scheduler_block_on((void *)notif);
    spin_lock(&notif->lock);
  }

  spin_unlock_irqrestore(&notif->lock, flags);
}

ErrorCode notif_wait_timeout(Notif *notif, u32 timeout_ms) {
  u64 deadline = timer_get_ticks() + (u64)timeout_ms * (CLOCK_HZ / 1000U);
  u64 flags = spin_lock_irqsave(&notif->lock);

  while (!notif->notified) {
    u64 now = timer_get_ticks();
    if (now >= deadline) {
      spin_unlock_irqrestore(&notif->lock, flags);
      return ERR_GEN_TIMEOUT;
    }
    u32 remaining_ms = (u32)((deadline - now) / (CLOCK_HZ / 1000U));

    spin_unlock(&notif->lock);
    scheduler_block_on_timeout((void *)notif, remaining_ms);
    spin_lock(&notif->lock);
  }

  spin_unlock_irqrestore(&notif->lock, flags);
  return SUCCESS;
}

void notif_signal(Notif *notif) {
  u64 flags = spin_lock_irqsave(&notif->lock);
  notif->notified = 1;
  spin_unlock_irqrestore(&notif->lock, flags);

  scheduler_wake_chan((void *)notif);
}

void notif_reset(Notif *notif) {
  u64 flags = spin_lock_irqsave(&notif->lock);
  notif->notified = 0;
  spin_unlock_irqrestore(&notif->lock, flags);
}
