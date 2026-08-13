# utils — Kernel Synchronization & IPC Primitives

Foundational blocking primitives for the kernel: semaphore, mutex, notify (one-shot
completion), condvar-style wait/notify, and a blocking pipe. These are a **prerequisite** for
the async Bluetooth work (see `RPI_Bluetooth/Docs/`) and for any future blocking driver/IPC.

## The core realization

Semaphore, mutex, notify, condvar, and the blocking pipe are all just **policy wrapped around
one scheduler capability**: *block the current task on some object, and wake tasks waiting on
that object.* That capability doesn't exist yet. So the true prerequisite is a **wait
primitive in the scheduler** — everything else is thin on top.

With `NUM_TASKS == 10`, we don't need linked wait-queues. The classic Unix "sleep on a
channel" (`wchan`) is enough and tiny:

```c
void scheduler_block_on(void *chan);          /* current → TASK_BLOCKED, wchan=chan, yield */
void scheduler_block_on_timeout(void *chan, u32 ms);
void scheduler_wake_chan(void *chan);         /* scan task[]: any blocked on chan → RUNNING */
```

Each primitive picks an address (usually the object itself) as its channel.

## Existing stubs (this is a fill-in, not a greenfield)

| File | State today | This effort |
|------|-------------|-------------|
| `inc/spinlock.h` + `src/spinlock.c` | **Done** — real `spin_lock`/`spin_unlock` (+ irqsave) | reuse as the low-level guard |
| `inc/semaphore.h` | empty (only `#include "common.h"`); `.c` empty | define `Semaphore` + counting API |
| `inc/mutex.h` | struct + `mutex_init/lock/unlock` decls; `.c` empty | implement blocking lock + trylock; reconcile struct |
| `inc/notify.h` | `Notif{notified}` + `notif_init/wait/signal`; `.c` empty | implement one-shot completion (+ timeout) |
| `inc/pipe.h` + `src/pipe.c` | implemented but **non-blocking** | make blocking via two wait-channels + mutex |
| `inc/shared_memory.*` | empty | **deferred** — belongs to the MMU/user-VM track |

Convention cleanup along the way: `notify.h`/`pipe.h` need the doxygen file-header + grouped
includes that `mutex.h`/`spinlock.h` already have; normalize `uint32_t` → `u32`; reconcile
`struct thread *` → `struct TaskBlock *`.

## Scope

**In (Tier 1 + pipe):** wait-channel core, semaphore, mutex, notify, condvar-style wait, pipe.
**Out:** shared memory — in a single flat EL1 address space it's only meaningful with the
MMU / per-process page-table work, so it's tracked on the memory-management/user-space effort,
not here.

## Document index

| Doc | Contents |
|-----|----------|
| [00_waitqueue_core.md](00_waitqueue_core.md) | Scheduler `block_on`/`wake_chan` + `TaskBlock` wait field |
| [01_semaphore.md](01_semaphore.md) | Counting semaphore |
| [02_mutex.md](02_mutex.md) | Blocking mutex + trylock; reconcile the existing struct |
| [03_notify.md](03_notify.md) | One-shot `Notif` completion (+ timeout) — BT's cmd-complete primitive |
| [04_pipe.md](04_pipe.md) | Turn the existing pipe blocking |
| [05_verification.md](05_verification.md) | IRQ/lock rules, lost-wakeup, tests |

## Phasing

- **S0** — wait-channel core in the scheduler (`00`). Prereq for all of the below.
- **S1** — semaphore + mutex (`01`, `02`).
- **S2** — notify (`03`). BT can start once this lands.
- **S3** — blocking pipe (`04`).

Each builds `-Werror` and boots in QEMU. BT's Phase 0 (`RPI_Bluetooth/Docs/`) becomes "consume
these" rather than rolling its own `Completion`.

## Conventions

Doxygen file-header + grouped includes on `.c` and `.h`; no `goto` (single-exit via static
`*_locked` helpers); `log()` has no `%lu`; short human comments.
