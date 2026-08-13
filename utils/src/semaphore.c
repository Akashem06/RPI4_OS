/*******************************************************************************************************************************
 * @file   semaphore.c
 *
 * @brief  Counting semaphore built on the scheduler wait-channel core
 *
 * @date   2026-08-05
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "scheduler.h"
#include "timer.h"

/* Intra-component Headers */
#include "semaphore.h"

/* The semaphore's count address doubles as its wait channel. IRQs stay masked from the
   count test through scheduler_block_on so a post() can never slip in and be lost. */

void semaphore_init(Semaphore *sem, int initial) {
  sem->count = initial;
  sem->lock.lock = 0;
}

void semaphore_wait(Semaphore *sem) {
  u64 flags = spin_lock_irqsave(&sem->lock);

  while (sem->count <= 0) {
    spin_unlock(&sem->lock); /* drop the lock but keep IRQs masked across the block */
    scheduler_block_on((void *)&sem->count);
    spin_lock(&sem->lock);
  }

  sem->count--;
  spin_unlock_irqrestore(&sem->lock, flags);
}

ErrorCode semaphore_wait_timeout(Semaphore *sem, u32 timeout_ms) {
  u64 deadline = timer_get_ticks() + (u64)timeout_ms * (CLOCK_HZ / 1000U);
  u64 flags = spin_lock_irqsave(&sem->lock);

  while (sem->count <= 0) {
    u64 now = timer_get_ticks();
    if (now >= deadline) {
      spin_unlock_irqrestore(&sem->lock, flags);
      return ERR_GEN_TIMEOUT;
    }
    u32 remaining_ms = (u32)((deadline - now) / (CLOCK_HZ / 1000U));

    spin_unlock(&sem->lock);
    scheduler_block_on_timeout((void *)&sem->count, remaining_ms);
    spin_lock(&sem->lock);
  }

  sem->count--;
  spin_unlock_irqrestore(&sem->lock, flags);
  return SUCCESS;
}

bool semaphore_try_wait(Semaphore *sem) {
  bool got = false;
  u64 flags = spin_lock_irqsave(&sem->lock);

  if (sem->count > 0) {
    sem->count--;
    got = true;
  }

  spin_unlock_irqrestore(&sem->lock, flags);
  return got;
}

void semaphore_post(Semaphore *sem) {
  u64 flags = spin_lock_irqsave(&sem->lock);
  sem->count++;
  spin_unlock_irqrestore(&sem->lock, flags);

  scheduler_wake_chan((void *)&sem->count);
}
