/*******************************************************************************************************************************
 * @file   pipe.c
 *
 * @brief  Blocking byte-pipe built on the scheduler wait-channel core
 *
 * @date   2026-08-05
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>

/* Inter-component Headers */
#include "scheduler.h"

/* Intra-component Headers */
#include "pipe.h"

/* Two wait channels sit on the pipe: readers sleep on &read_pos (woken when data arrives),
   writers sleep on &write_pos (woken when space frees). A short spinlock (not a sleeping
   mutex) guards the indices so the test-and-block window stays lost-wakeup safe. */

ErrorCode pipe(Pipe *pipe) {
  pipe->write_pos = 0;
  pipe->read_pos = 0;
  pipe->closed = 0;
  pipe->lock.lock = 0;
  return SUCCESS;
}

ErrorCode pipe_read(Pipe *pipe, u8 *buff, u16 size) {
  if (buff == NULL) {
    return ERR_GEN_INVALID_PARAM;
  }
  if (size == 0) {
    return SUCCESS;
  }

  u64 flags = spin_lock_irqsave(&pipe->lock);

  while (pipe->read_pos == pipe->write_pos && !pipe->closed) {
    spin_unlock(&pipe->lock); /* keep IRQs masked across the block */
    scheduler_block_on((void *)&pipe->read_pos);
    spin_lock(&pipe->lock);
  }

  /* Drained and closed => end of stream */
  if (pipe->read_pos == pipe->write_pos && pipe->closed) {
    spin_unlock_irqrestore(&pipe->lock, flags);
    return SUCCESS;
  }

  u16 bytes_read = 0;
  while (bytes_read < size && pipe->read_pos != pipe->write_pos) {
    buff[bytes_read++] = pipe->buffer[pipe->read_pos];
    pipe->read_pos = (pipe->read_pos + 1) % PIPE_SIZE;
  }

  spin_unlock_irqrestore(&pipe->lock, flags);
  scheduler_wake_chan((void *)&pipe->write_pos); /* space freed for writers */
  return (ErrorCode)bytes_read;
}

ErrorCode pipe_write(Pipe *pipe, u8 *buff, u16 size) {
  if (buff == NULL) {
    return ERR_GEN_INVALID_PARAM;
  }

  u16 bytes_written = 0;
  u64 flags = spin_lock_irqsave(&pipe->lock);

  while (bytes_written < size) {
    if (pipe->closed) {
      spin_unlock_irqrestore(&pipe->lock, flags);
      /* Partial write still reports progress, a fully-broken pipe is an error */
      return (bytes_written > 0) ? (ErrorCode)bytes_written : ERR_SYS_INVALID_OP;
    }

    u32 next = (pipe->write_pos + 1) % PIPE_SIZE;
    if (next == pipe->read_pos) {
      /* Full: hand the reader what we have so it can drain, then wait for space */
      spin_unlock(&pipe->lock);
      scheduler_wake_chan((void *)&pipe->read_pos);
      scheduler_block_on((void *)&pipe->write_pos);
      spin_lock(&pipe->lock);
      continue;
    }

    pipe->buffer[pipe->write_pos] = buff[bytes_written++];
    pipe->write_pos = next;
  }

  spin_unlock_irqrestore(&pipe->lock, flags);
  scheduler_wake_chan((void *)&pipe->read_pos); /* new data for readers */
  return (ErrorCode)bytes_written;
}

ErrorCode pipe_close(Pipe *pipe) {
  u64 flags = spin_lock_irqsave(&pipe->lock);
  pipe->closed = 1;
  spin_unlock_irqrestore(&pipe->lock, flags);

  /* Unblock both ends: readers drain to EOF, writers bail out */
  scheduler_wake_chan((void *)&pipe->read_pos);
  scheduler_wake_chan((void *)&pipe->write_pos);
  return SUCCESS;
}
