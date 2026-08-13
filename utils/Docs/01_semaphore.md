# S1a — Counting semaphore

The simplest wchan client. `semaphore.h` is currently empty (only `#include "common.h"`); this
defines the struct + API.

## API

File: `utils/inc/semaphore.h` + `utils/src/semaphore.c`.

```c
typedef struct {
  volatile i32    count;   /* available permits; may go to what init sets */
  struct Spinlock lock;    /* guards count + the test-and-block window */
} Semaphore;

void      semaphore_init(Semaphore *s, i32 initial);
void      semaphore_wait(Semaphore *s);                    /* down / P — blocks if count == 0 */
ErrorCode semaphore_wait_timeout(Semaphore *s, u32 ms);    /* SUCCESS | ERR_GEN_TIMEOUT */
bool      semaphore_try_wait(Semaphore *s);                /* down if available, else false */
void      semaphore_post(Semaphore *s);                    /* up / V — wakes one waiter */
```

## Implementation

Channel = the semaphore address. Test-and-block under the guard to avoid the lost-wakeup race:

```c
void semaphore_wait(Semaphore *s) {
  for (;;) {
    spin_lock_irqsave(&s->lock, &flags);
    if (s->count > 0) { s->count--; spin_unlock_irqrestore(&s->lock, flags); return; }
    spin_unlock_irqrestore(&s->lock, flags);
    scheduler_block_on(s);            /* re-loops and re-tests after wake */
  }
}

void semaphore_post(Semaphore *s) {
  spin_lock_irqsave(&s->lock, &flags);
  s->count++;
  spin_unlock_irqrestore(&s->lock, flags);
  scheduler_wake_chan(s);            /* wake blocked waiters; they re-test count */
}
```

The re-test loop makes the wake edge-tolerant: a spurious or shared wake just re-checks `count`.
`wake_chan` wakes all blocked-on-`s` tasks; each re-competes for a permit under the lock, so
counting semantics hold even with multiple waiters.

`semaphore_wait_timeout` uses `scheduler_block_on_timeout` and returns `ERR_GEN_TIMEOUT` if the
deadline passes without a permit.

## Notes

- `semaphore_post` is safe to call from a worker thread or an ISR (it only touches `count`
  under an irqsave lock, then wakes).
- This is the building block for the mutex (binary case) and can back the BT worker-wakeup if a
  counting signal is ever preferable to a one-shot `Notif`.

## Verification

Producer/consumer: init `count=0`; consumer `semaphore_wait` blocks; producer `semaphore_post`
N times; assert the consumer runs exactly N times. Timeout case: `semaphore_wait_timeout` with
no post returns `ERR_GEN_TIMEOUT`. Confirm idle thread keeps ticking while blocked.
