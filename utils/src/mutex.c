/*******************************************************************************************************************************
 * @file   mutex.c
 *
 * @brief  Blocking recursive mutex built on the scheduler wait-channel core
 *
 * @date   2025-01-05
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>

/* Inter-component Headers */
#include "scheduler.h"

/* Intra-component Headers */
#include "mutex.h"

/* The mutex address is its own wait channel. IRQs stay masked from the state test through
   scheduler_block_on so an unlock() can't race an about-to-block waiter. */

void mutex_init(Mutex *mutex) {
  mutex->state = 0;
  mutex->owner = NULL;
  mutex->lock_count = 0;
  mutex->lock.lock = 0;
}

bool mutex_lock(Mutex *mutex) {
  u64 flags = spin_lock_irqsave(&mutex->lock);

  /* Already ours, just bump the recursion depth */
  if (mutex->state == 1 && mutex->owner == current) {
    mutex->lock_count++;
    spin_unlock_irqrestore(&mutex->lock, flags);
    return true;
  }

  while (mutex->state == 1) {
    spin_unlock(&mutex->lock); /* keep IRQs masked across the block */
    scheduler_block_on((void *)mutex);
    spin_lock(&mutex->lock);
  }

  mutex->state = 1;
  mutex->owner = current;
  mutex->lock_count = 1;
  spin_unlock_irqrestore(&mutex->lock, flags);
  return true;
}

bool mutex_trylock(Mutex *mutex) {
  bool got = false;
  u64 flags = spin_lock_irqsave(&mutex->lock);

  if (mutex->state == 0) {
    mutex->state = 1;
    mutex->owner = current;
    mutex->lock_count = 1;
    got = true;
  } else if (mutex->owner == current) {
    mutex->lock_count++;
    got = true;
  }

  spin_unlock_irqrestore(&mutex->lock, flags);
  return got;
}

bool mutex_unlock(Mutex *mutex) {
  u64 flags = spin_lock_irqsave(&mutex->lock);

  if (mutex->state == 0 || mutex->owner != current) {
    spin_unlock_irqrestore(&mutex->lock, flags);
    return false;
  }

  if (--mutex->lock_count > 0) {
    /* Still held by us at an outer level */
    spin_unlock_irqrestore(&mutex->lock, flags);
    return true;
  }

  mutex->state = 0;
  mutex->owner = NULL;
  spin_unlock_irqrestore(&mutex->lock, flags);

  scheduler_wake_chan((void *)mutex);
  return true;
}
