#pragma once

#include "common.h"
#include "error.h"

#define PIPE_SIZE 512

typedef struct {
  u32 write_pos;
  u32 read_pos;
  u8 buffer[PIPE_SIZE];
  u8 closed;
} Pipe;

ErrorCode pipe(Pipe *pipe);
ErrorCode pipe_read(Pipe *pipe, u8 *buff, u16 size);
ErrorCode pipe_write(Pipe *pipe, u8 *buff, u16 size);
ErrorCode pipe_close(Pipe *pipe);