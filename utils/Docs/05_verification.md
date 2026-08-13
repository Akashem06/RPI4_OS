# 05 — Concurrency rules & verification

## The three-layer locking model

1. **Spinlock** (`utils/spinlock.h`) — short, non-sleeping critical sections. The *only* lock
   usable inside the primitives themselves and inside ISRs. Use the `irqsave` variants when the
   section can also run from, or race with, an interrupt.
2. **Wait-channel core** (`scheduler_block_on`/`wake_chan`) — the sleep/wake mechanism. Not a
   lock; the mechanism the blocking primitives are built on.
3. **Blocking primitives** (semaphore/mutex/notify/pipe) — longer sections where sleeping is OK.
   Built from (1) guarding the test-and-block window and (2) doing the actual sleep.

Rules:
- Never `mutex_lock` / `semaphore_wait` / `notif_wait` / `pipe_read` from an ISR — they can
  block. ISRs only `*_signal` / `*_post` / `wake_chan` (all irqsave + wake, non-blocking).
- Never hold a spinlock across a call that can block.
- `*_signal`/`*_post`/`wake_chan` are safe from thread or ISR context.

## The lost-wakeup invariant (applies to every primitive)

Each blocking primitive must **test its condition and commit to blocking under the same guard**,
releasing the guard only as it blocks, and **re-test after waking**:

```
lock;
if (condition_satisfied) { consume; unlock; return; }
unlock;
scheduler_block_on(chan);   /* wake re-enters the loop and re-tests */
```

Because the signaller sets the condition under the same lock *before* `wake_chan`, a wake that
races an about-to-block waiter is observed on the re-test, never lost. The re-test loop also
makes wakes idempotent (a shared/spurious wake just re-checks and blocks again).

## Timeouts

`scheduler_block_on_timeout` gives a deadline-based wake. Wrappers return `ERR_GEN_TIMEOUT`
when the deadline passes with the condition still unmet. Every primitive that BT depends on for
liveness (notify command-complete, semaphore) exposes a `_timeout` variant so a missing
signal/reply can't wedge a thread.

## Verification harness (all in QEMU)

Reusable two-kthread pattern via `scheduler_create_task(PF_KTHREAD, ...)`:

| Primitive | Test | Pass criteria |
|-----------|------|---------------|
| wchan core | A blocks on `&x`, B wakes it | B→A ordering; idle keeps ticking while A blocked |
| semaphore | producer posts N, consumer waits | consumer runs exactly N times; timeout variant fires |
| mutex | two threads increment shared counter under mutex | no lost updates; recursive lock/unlock; trylock=false when held |
| notify | pre-signal + normal + timeout | sticky pre-signal returns immediately; timeout returns `ERR_GEN_TIMEOUT` |
| pipe | write > PIPE_SIZE, slow reader; close-while-blocked | all bytes in order, none lost; blocked end unblocks; reader sees EOF |

Cross-cutting liveness check in **every** test: a separate idle/counter kthread must keep
running while another thread is blocked — proves the primitive sleeps (yields the CPU) instead
of spinning, and that the scheduler never wedges when all real threads are blocked.

Build each step `-Werror`; boot both the default mem-test config and a
`kernel/examples/sync_test.c` demo (per the project build/verify env).

## Follow-ups (not in this effort)

- **Priority inheritance** for the mutex (the `original_priority` field) — needs scheduler
  priority-set; revisit if priority inversion shows up.
- **Precise timer-driven timeouts** (arm a timer to `wake_chan`) if deadline-on-wake proves too
  coarse.
- **Shared memory** — deferred to the MMU/user-VM track (meaningless in one flat address space).
- **fd-backed pipes** via `DeviceOps` + the fd syscalls, once a use case appears.
