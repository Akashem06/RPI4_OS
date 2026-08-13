#pragma once

/*******************************************************************************************************************************
 * @file   pipe.h
 *
 * @brief  Blocking byte-pipe API
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

#define PIPE_SIZE 512

/**
 * @brief   Single-reader / single-writer blocking byte pipe
 * @details One slot is kept empty to tell "full" from "empty". Readers block until data is
 *          available, writers block until space frees up, and closing the pipe unblocks both
 *          ends (readers then drain and see EOF).
 */
typedef struct {
  u32 write_pos;          /**< Producer index */
  u32 read_pos;           /**< Consumer index */
  u8 buffer[PIPE_SIZE];   /**< Ring storage */
  u8 closed;              /**< Set by pipe_close */
  struct Spinlock lock;   /**< Guards the indices and the test-and-block windows */
} Pipe;

/**
 * @brief   Initialize a pipe to the empty, open state
 * @param   pipe Pointer to a pipe struct
 * @return  SUCCESS
 */
ErrorCode pipe(Pipe *pipe);

/**
 * @brief   Read from the pipe, blocking until at least one byte is available or it closes
 * @param   pipe Pointer to a pipe struct
 * @param   buff Destination buffer
 * @param   size Maximum bytes to read
 * @return  Bytes read (>0), 0 at end-of-stream (closed and drained), or a negative ErrorCode
 */
ErrorCode pipe_read(Pipe *pipe, u8 *buff, u16 size);

/**
 * @brief   Write to the pipe, blocking until all bytes are queued or it closes
 * @param   pipe Pointer to a pipe struct
 * @param   buff Source buffer
 * @param   size Bytes to write
 * @return  Bytes written, or a negative ErrorCode
 */
ErrorCode pipe_write(Pipe *pipe, u8 *buff, u16 size);

/**
 * @brief   Close the pipe and wake both ends
 * @param   pipe Pointer to a pipe struct
 * @return  SUCCESS
 */
ErrorCode pipe_close(Pipe *pipe);

/** @} */
