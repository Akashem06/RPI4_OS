# S3 — Blocking pipe

`pipe.c` is already implemented as a byte ring, but it's **non-blocking** — `pipe_read` returns
whatever's available (or 0) immediately, and `pipe_write` returns short on a full buffer. Turn
it into a proper blocking pipe using two wait-channels + a mutex.

## Current state (`utils/inc/pipe.h`, `utils/src/pipe.c`)

```c
typedef struct { u32 write_pos; u32 read_pos; u8 buffer[PIPE_SIZE]; u8 closed; } Pipe;
ErrorCode pipe(Pipe *pipe);
ErrorCode pipe_read (Pipe *pipe, u8 *buff, u16 size);   /* returns immediately if empty */
ErrorCode pipe_write(Pipe *pipe, u8 *buff, u16 size);   /* returns short if full */
ErrorCode pipe_close(Pipe *pipe);
```

The ring index math is already correct; what's missing is blocking + concurrency safety. Also
lacks the doxygen file-header block; `pipe(Pipe*)` is declared but unimplemented (should be
`pipe_init`).

## Target

```c
typedef struct {
  u32 write_pos, read_pos;
  u8  buffer[PIPE_SIZE];
  u8  closed;
  struct Spinlock lock;   /* guards indices + closed */
  /* wait channels are addresses within the struct: &can_read, &can_write */
  u32 can_read, can_write;
} Pipe;

ErrorCode pipe_init(Pipe *p);
i32       pipe_read (Pipe *p, u8 *buf, u16 size);   /* blocks until >=1 byte or closed; returns count (0 = EOF) */
i32       pipe_write(Pipe *p, const u8 *buf, u16 size); /* blocks until all written or closed */
ErrorCode pipe_close(Pipe *p);                      /* marks closed, wakes all waiters */
```

## Implementation

Two channels: readers sleep on `&p->can_read`, writers on `&p->can_write`.

- `pipe_read`: under `lock`, if empty and not closed → unlock, `scheduler_block_on(&p->can_read)`,
  retry. If closed and empty → return 0 (EOF). Copy available bytes, advance `read_pos`, unlock,
  `scheduler_wake_chan(&p->can_write)` (space freed).
- `pipe_write`: under `lock`, if full and not closed → unlock, `scheduler_block_on(&p->can_write)`,
  retry. If closed → return error/`ERR_IPC_PIPE`. Copy bytes, advance `write_pos`, unlock,
  `scheduler_wake_chan(&p->can_read)` (data available).
- `pipe_close`: set `closed`, `scheduler_wake_chan(&p->can_read)` **and** `&p->can_write` so
  blocked ends unblock and observe EOF/error.

Keep one-byte-reserved fullness (as the current code does: `space = ... - 1`) so read==write
means empty unambiguously.

## Notes

- This composes the wait-channel core + a mutex; it's a *client* of the primitives, not a new
  primitive — hence its own phase after S0–S2.
- Return type moves to `i32` so short/blocking counts and negative `ErrorCode`s coexist (matches
  the pattern used elsewhere: bytes on success, negative on error).
- Optionally back-fill a `pipe` device (`DeviceOps` read/write) later so pipes are usable via
  the fd syscalls; not required now.

**Implemented (2026-08-05):** the guard is a **`struct Spinlock`, not a sleeping mutex.** You
can't hold a sleeping mutex across `scheduler_block_on` without a condvar (the release→block gap
would drop wakeups); the short IRQ-masked spinlock is the correct guard and keeps the
test-and-block window lost-wakeup safe, exactly like the other primitives. No separate
`can_read`/`can_write` fields were needed — readers sleep on `&p->read_pos`, writers on
`&p->write_pos` (any stable address works as a channel). One extra detail vs. the sketch: when a
writer fills the buffer it must `wake_chan(&read_pos)` **before** blocking on full, or a reader
that parked while the pipe was empty would never be woken to drain it (deadlock). Kept the
existing `ErrorCode` return type rather than adding `i32`; counts are non-negative and fit.

## Verification

Producer kthread `pipe_write`s more than `PIPE_SIZE` bytes; consumer `pipe_read`s slowly →
producer blocks on full and resumes as the consumer drains; all bytes arrive in order, none
lost. Close case: reader blocked on empty, writer `pipe_close` → reader returns 0 (EOF).
