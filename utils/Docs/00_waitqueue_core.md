# S0 — Wait-channel core in the scheduler

The single capability every blocking primitive needs: put the current task to sleep on a
channel, and wake tasks sleeping on that channel. Everything else (`semaphore`, `mutex`,
`notify`, `pipe`) is a thin wrapper.

## Why "sleep on a channel" (wchan) and not wait-queues

Linked wait-queues need a list node per waiter and careful list surgery. With `NUM_TASKS == 10`
a wake is just a linear scan of `task[]` — O(10), trivial, and it needs **no** per-object list
storage. This is the classic Unix `sleep()/wakeup()` model. If the task table ever grows large
we can swap the scan for intrusive lists behind the same API.

## TaskBlock change

File: `lib/inc/scheduler.h`. Add a wait channel to the task:

```c
struct TaskBlock {
  struct CPUContext cpu_context;
  long state;
  long counter;
  long priority;
  long preempt_count;
  unsigned long stack;
  unsigned long flags;
  void *wait_channel;   /* NEW: non-NULL while blocked on a channel */
};
```

**Also update `INIT_TASK`** (the positional initializer macro in scheduler.h) to add a trailing
`0` for `wait_channel`, or the idle task's field is uninitialized.

## API

File: `lib/inc/scheduler.h` + `lib/src/scheduler.c`.

```c
void scheduler_block_on(void *chan);
void scheduler_block_on_timeout(void *chan, u32 timeout_ms);  /* woken by wake or deadline */
void scheduler_wake_chan(void *chan);                          /* wake all blocked on chan */
```

Sketch:

```c
void scheduler_block_on(void *chan) {
  preempt_disable();
  current->wait_channel = chan;
  current->state = TASK_BLOCKED;
  current->counter = 0;
  preempt_enable();
  schedule();                       /* yields; returns once woken */
  current->wait_channel = NULL;
}

void scheduler_wake_chan(void *chan) {
  u64 flags = irq_save_flags();
  irq_disable();
  for (int i = 0; i < NUM_TASKS; i++) {
    struct TaskBlock *t = task[i];
    if (t && t->state == TASK_BLOCKED && t->wait_channel == chan) {
      t->state = TASK_RUNNING;
      if (t->counter <= 0) t->counter = t->priority;
    }
  }
  irq_restore_flags(flags);
}
```

Why it's safe with the current scheduler:
- `pick_next_task` already skips non-`TASK_RUNNING` tasks → a blocked task isn't selected; no
  core change.
- `task[0]`/init_task is always `TASK_RUNNING` → there's always a runnable idle task, so
  "everyone blocked" doesn't hang the CPU.
- `scheduler_wake_chan` uses the same `irq_save_flags`/`irq_disable` shape as
  `scheduler_tick_handler`, so it's safe from a worker thread or an ISR.

## The wake-before-block race (important)

A wakeup that fires *between* a caller testing its condition and calling `scheduler_block_on`
would be lost. The core doesn't solve this alone — **the wrapping primitive must test its
condition and block under a guard** (`spin_lock_irqsave` / `preempt_disable`), releasing it only
as it blocks. Every primitive doc (semaphore/mutex/notify/pipe) follows this pattern; see
`05_verification.md`.

## Timeout

`scheduler_block_on_timeout` records a deadline from `timer_get_ticks()`. Minimal version:
on each wake, the wrapper re-checks its condition *and* the deadline. A later refinement arms a
timer to `scheduler_wake_chan(chan)` at the deadline for precise timeouts. Deadline-on-wake is
enough to start (and enough for BT command timeouts, which also get woken by the reply).

**Implemented (2026-08-05):** `TaskBlock` gained `void *wait_channel` **and** `u64 wake_deadline`.
The deadline scan (`wake_expired_timeouts`) lives at the top of `_schedule()`, not only in
`scheduler_tick_handler` — because QEMU raspi4b never delivers the timer tick (secure PPI,
unreachable at NS-EL2), so a tick-only timeout would never fire in sim. Running it from the
scheduling path means the cooperative idle loop (`while (1) schedule()`) drives timeouts too.
`timer_get_ticks()` reads the free-running 1 MHz counter, which advances with or without the
tick. `block_current_on` keeps IRQs masked across `_schedule()` (mirroring the tick handler) and
restores them on resume.

## Verification

Two kthreads: A `scheduler_block_on(&x)` then logs; B ticks a few times, logs, then
`scheduler_wake_chan(&x)`. Expect B→A ordering, and confirm the idle thread keeps ticking while
A is blocked. Add a timeout case: A `scheduler_block_on_timeout(&x, 500)` with no waker → A
resumes after ~500 ms.
