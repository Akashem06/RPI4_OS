#pragma once

#include "common.h"
#include "errors.h"

#define PIPE_SIZE 512

typedef struct {
    u32 write_pos;
    u32 read_pos;
    u8 buffer[PIPE_SIZE];
    u8 closed;
} Pipe;

error_os pipe(Pipe *pipe);
error_os pipe_read(Pipe *pipe, u8 *buff, u16 size);
error_os pipe_write(Pipe *pipe, u8 *buff, u16 size);
error_os pipe_close(Pipe *pipe);