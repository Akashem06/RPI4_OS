#include "pipe.h"
#include <stddef.h>

error_os pipe(Pipe *pipe);

error_os pipe_read(Pipe *pipe, u8 *buff, u16 size) {
    if (buff == NULL) {
        return -1;
    }
    if (pipe->closed) {
        return -1;
    }

    u16 bytes_read = 0;

    while (size) {
        if (pipe->read_pos == pipe->write_pos) {
            return bytes_read; /* Not enough data in the pipe */
        }
        buff[bytes_read++] = pipe->buffer[pipe->read_pos];
        pipe->read_pos = (pipe->read_pos + 1) % PIPE_SIZE;
        size--;
    }

    return bytes_read;
}

error_os pipe_write(Pipe *pipe, u8 *buff, u16 size) {
    if (buff == NULL) {
        return -1;
    }
    if (pipe->closed) {
        return -1;
    }

    u16 bytes_written = 0;
    u16 space_available;

    while (size) {
        if (pipe->write_pos >= pipe->read_pos) {
            space_available = PIPE_SIZE - pipe->write_pos + pipe->read_pos - 1;
        } else {
            space_available = pipe->read_pos - pipe->write_pos - 1;
        }

        if (space_available == 0) {
            return bytes_written;
        }

        pipe->buffer[pipe->write_pos] = buff[bytes_written++];
        pipe->write_pos = (pipe->write_pos + 1) % PIPE_SIZE;
        size--;
    }

    return bytes_written;
}

error_os pipe_close(Pipe *pipe) {
    pipe->closed = 1;
    return 0;
}
