# S1b — Blocking mutex

`mutex.h` already declares a struct + `mutex_init/lock/unlock`, but `mutex.c` is empty and the
current signatures/fields need reconciling.

## Reconcile the existing header

Today (`utils/inc/mutex.h`):

```c
typedef struct {
  volatile u32 state;         /* 0 = unlocked, 1 = locked */
  // TaskId owner;
  u32 lock_count;             /* recursive support */
  u32 original_priority;      /* priority-inheritance hint */
  struct thread *waiting;     /* list of waiting threads  <-- struct thread doesn't exist */
} Mutex;

void mutex_init(Mutex *mutex);
bool mutex_lock(Mutex *mutex);    /* comment reads like a try-lock */
bool mutex_unlock(Mutex *mutex);
```

Problems: `struct thread` isn't the scheduler type (`struct TaskBlock`); `mutex_lock` returning
`bool` reads like *try*, but a real mutex should **block**; `waiting` list is unnecessary with
the wchan core.

Proposed reconciliation:

```c
typedef struct {
  volatile u32      state;        /* 0 = unlocked, 1 = locked */
  struct TaskBlock *owner;        /* current holder, or NULL */
  u32               lock_count;   /* recursive acquisitions by owner */
  struct Spinlock   lock;         /* guards the above + block window */
} Mutex;

void mutex_init(Mutex *m);
void mutex_lock(Mutex *m);        /* blocking acquire (recursive for the owner) */
bool mutex_trylock(Mutex *m);     /* non-blocking: true if acquired */
void mutex_unlock(Mutex *m);      /* only the owner may unlock */
```

Drop `waiting` (wchan scans `task[]`). Keep `lock_count` for recursion. `original_priority` is
only needed for **priority inheritance** — leave it out for now and note PI as a follow-up
(see below); don't carry a dead field.

## Implementation

Channel = the mutex address. Binary-semaphore logic + ownership:

```c
void mutex_lock(Mutex *m) {
  for (;;) {
    spin_lock_irqsave(&m->lock, &flags);
    if (m->state == 0) { m->state = 1; m->owner = current; m->lock_count = 1;
                         spin_unlock_irqrestore(&m->lock, flags); return; }
    if (m->owner == current) { m->lock_count++;                      /* recursive */
                         spin_unlock_irqrestore(&m->lock, flags); return; }
    spin_unlock_irqrestore(&m->lock, flags);
    scheduler_block_on(m);
  }
}

void mutex_unlock(Mutex *m) {
  spin_lock_irqsave(&m->lock, &flags);
  if (m->owner != current) { spin_unlock_irqrestore(&m->lock, flags); BUG("mutex: bad unlock"); }
  if (--m->lock_count == 0) { m->state = 0; m->owner = NULL; }
  spin_unlock_irqrestore(&m->lock, flags);
  if (m->lock_count == 0) scheduler_wake_chan(m);
}
```

`mutex_trylock` is the non-block fast path (the current `bool mutex_lock` intent), returning
`false` instead of blocking.

## Notes / follow-ups

- **Mutex vs spinlock:** spinlock for short, non-sleeping critical sections (and the only thing
  usable *inside* these primitives); mutex for longer sections where sleeping is fine. Don't
  hold a spinlock across a `mutex_lock`.
- **Priority inheritance** (the `original_priority` hint): deferred. Needs the mutex to bump the
  owner's priority to the max waiter's while held. Track it once the scheduler exposes
  priority-set; not required for BT.
- **ISR rule:** never `mutex_lock` from an ISR (it can block). ISRs use spinlocks / `wake_chan`.

## Verification

Two kthreads contend on one mutex around a shared counter; assert no lost updates and that the
blocked thread yields (idle keeps ticking). Recursive case: owner locks twice, unlocks twice,
waiter only proceeds after the second unlock. `mutex_trylock` returns false when held.
