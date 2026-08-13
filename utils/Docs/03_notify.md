# S2 — One-shot notify (completion)

`notify.h` already declares exactly the primitive the async BT layer needs for command
completion: a one-shot "wait until signalled." This is BT's "Completion" — **we do not build a
separate `lib/completion.c`; BT uses `Notif`.**

## Reconcile the existing header

Today (`utils/inc/notify.h`) — note it lacks the project's doxygen file-header block and uses
`uint32_t`:

```c
typedef struct { volatile uint32_t notified; } Notif;
void notif_init(Notif *notif);
void notif_wait(Notif *notif);
void notif_signal(Notif *notif);
```

Proposed (add file-header + grouped includes; `u32`; add a spinlock + timeout):

```c
typedef struct {
  volatile u32    notified;   /* 0 until signalled; sticky (one-shot) */
  struct Spinlock lock;
} Notif;

void      notif_init(Notif *n);
void      notif_wait(Notif *n);                    /* returns immediately if already signalled */
ErrorCode notif_wait_timeout(Notif *n, u32 ms);    /* SUCCESS | ERR_GEN_TIMEOUT */
void      notif_signal(Notif *n);                  /* sticky: sets notified, wakes waiter(s) */
void      notif_reset(Notif *n);                   /* re-arm for reuse */
```

Sticky semantics matter: a `signal` that arrives *before* `wait` must still be seen (the reply
can beat the waiter). `notified` stays set until `notif_reset`.

## Implementation

Channel = the notif address. Test-and-block under the guard:

```c
ErrorCode notif_wait_timeout(Notif *n, u32 ms) {
  for (;;) {
    spin_lock_irqsave(&n->lock, &flags);
    if (n->notified) { spin_unlock_irqrestore(&n->lock, flags); return SUCCESS; }
    spin_unlock_irqrestore(&n->lock, flags);
    if (scheduler_block_on_timeout(n, ms) == TIMED_OUT && !n->notified) return ERR_GEN_TIMEOUT;
  }
}

void notif_signal(Notif *n) {
  spin_lock_irqsave(&n->lock, &flags);
  n->notified = 1;
  spin_unlock_irqrestore(&n->lock, flags);
  scheduler_wake_chan(n);
}
```

`notif_signal` is ISR-safe (irqsave + wake_chan), so the UART IRQ can signal the BT worker.

## Where BT uses it

- **Command completion:** `HCI_send_command` holds a `Notif` per outstanding command;
  `notif_wait_timeout(&pending.done, HCI_CMD_TIMEOUT_MS)`; the worker `notif_signal`s it when
  the matching Command Complete/Status arrives. Replaces the `while(waiting_response){}` spin.
- **Worker wakeup:** the UART IRQ `notif_signal`s the worker's wakeup `Notif` after pushing
  bytes into the ring; the worker `notif_wait`s (or `notif_wait_timeout` for an idle tick) then
  `notif_reset`s each loop. (A counting `Semaphore` is an alternative if signal coalescing
  becomes an issue; `Notif` + drain-loop is fine because the worker always drains the whole
  ring.)

This is why BT's Phase 0 collapses to "consume `utils/` primitives."

## Verification

One kthread `notif_wait`s, another `notif_signal`s after N ticks → ordering holds. Pre-signal
case: `notif_signal` *before* the waiter calls `notif_wait` → wait returns immediately (proves
sticky). Timeout case: `notif_wait_timeout` with no signal → `ERR_GEN_TIMEOUT`.
