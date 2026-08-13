#include "scheduler.h"

#include <stdbool.h>
#include <stddef.h>

#include "assert.h"
#include "entry.h"
#include "irq.h"
#include "log.h"
#include "memops.h"
#include "page.h"
#include "sysregs.h"
#include "timer.h"

struct TaskBlock init_task = INIT_TASK;
static bool is_initialized = false;

/* Board-supplied periodic tick source that drives preemption */
static const struct SchedTickSource *tick_source = NULL;

__attribute__((aligned(8), section(".data"))) struct TaskBlock *current = NULL;
__attribute__((aligned(8), section(".data"))) struct TaskBlock *task[NUM_TASKS] = { NULL };
__attribute__((aligned(8), section(".data"))) u8 num_tasks = 0;

static inline long clamp_priority(long priority) {
  if (priority < MIN_PRIORITY) return MIN_PRIORITY;
  if (priority > MAX_PRIORITY) return MAX_PRIORITY;
  return priority;
}

void preempt_disable(void) {
  if (is_initialized == false) {
    return;
  }
  current->preempt_count++;
}

void preempt_enable(void) {
  if (is_initialized == false) {
    return;
  }
  current->preempt_count--;
}

void switch_to(struct TaskBlock *next) {
  if (current != next) {
    // A misaligned SP/LR means the task context is corrupt, switching would fault, bail hard
    if ((next->cpu_context.sp & 15) != 0) {
      BUG("switch_to: task SP not 16-byte aligned");
    }
    if ((next->cpu_context.lr & 3) != 0) {
      BUG("switch_to: task LR not 4-byte aligned");
    }

    struct TaskBlock *prev = current;
    current = next;
    cpu_context_switch(prev, next);
  }
}

static void update_task_timeslices(void) {
  struct TaskBlock *p;

  for (int i = 0; i < NUM_TASKS; i++) {
    p = task[i];
    if (!p) continue;

    // Calculate new counter values
    long new_counter = (p->counter >> 1) + p->priority;
    p->counter = max(MIN_TIMESLICE, new_counter);

    // Cap the counter to priority based maximum
    p->counter = min(p->counter, p->priority * 2);
  }
}

static int pick_next_task(void) {
  int next = 0;
  int max_counter = -1;
  volatile struct TaskBlock *p;
  bool found = false;

  for (int i = 0; i < NUM_TASKS; i++) {
    p = task[i];
    if (!p || p->state != TASK_RUNNING || p->counter == 0) {
      continue;
    }
    if (p->counter > max_counter) {
      max_counter = p->counter;
      next = i;
      found = true;
    }
  }

  if (found) {
    return next;
  }

  update_task_timeslices();

  for (int i = 0; i < NUM_TASKS; i++) {
    p = task[i];
    if (!p || p->state != TASK_RUNNING) {
      continue;
    }
    if (p->counter > max_counter) {
      max_counter = p->counter;
      next = i;
    }
  }

  return (max_counter > 0) ? next : -1;
}

static void wake_expired_timeouts(void);

void _schedule(void) {
  preempt_disable();
  int next;

  wake_expired_timeouts(); /* promote any timed-out blocked tasks before we pick */

  while (1) {
    next = pick_next_task();
    if (next >= 0) {
      break;
    }
    update_task_timeslices();
  }

  if (task[next] != current) {
    switch_to(task[next]);
  }

  preempt_enable();
}

void schedule() {
  if (is_initialized == false) {
    return;
  }
  if (current->preempt_count > 0) {
    return;
  }

  current->counter = 0;
  _schedule();
}

/* Wake any timed-out blocked task. IRQ-safe, runs from the scheduling path (see _schedule),
   so timeouts fire under cooperative scheduling too, not just the (QEMU-absent) timer tick. */
static void wake_expired_timeouts(void) {
  u64 flags = irq_save_flags();
  irq_disable();

  u64 now = timer_get_ticks();
  for (int i = 0; i < NUM_TASKS; i++) {
    struct TaskBlock *t = task[i];
    if (t && t->state == TASK_BLOCKED && t->wake_deadline != 0 && now >= t->wake_deadline) {
      t->state = TASK_RUNNING;
      t->wake_deadline = 0;
      if (t->counter <= 0) t->counter = t->priority;
    }
  }

  irq_restore_flags(flags);
}

void scheduler_tick_handler() {
  if (!current) {
    return;
  }

  u64 flags = irq_save_flags();
  irq_disable();

  if (--current->counter > 0 || current->preempt_count > 0) {
    irq_restore_flags(flags);
    return;
  }

  current->counter = 0;
  _schedule();

  irq_restore_flags(flags);
}

void scheduler_init() {
  /* init_task is task[0], the always-runnable idle thread, it runs the kernel idle loop */
  current = &init_task;
  current->state = TASK_RUNNING;
  task[0] = current;

  num_tasks = 1;

  is_initialized = true;
}

void scheduler_set_tick_source(const struct SchedTickSource *src) {
  tick_source = src;
}

ErrorCode scheduler_start_tick(u32 hz) {
  if (!tick_source || !tick_source->start) {
    return ERR_SYS_INVALID_OP;
  }
  return tick_source->start(hz);
}

/* Find a task-table slot, reclaiming a finished (zombie) task's pages if needed */
static int task_table_alloc_slot(void) {
  for (int i = 0; i < NUM_TASKS; i++) {
    if (task[i] == NULL) {
      return i;
    }
  }
  for (int i = 0; i < NUM_TASKS; i++) {
    if (task[i] && task[i]->state == TASK_ZOMBIE) {
      if (task[i]->stack) {
        free_page(task[i]->stack);
      }
      free_page((u64)task[i]);
      task[i] = NULL;
      num_tasks--;
      return i;
    }
  }
  return -1;
}

ErrorCode scheduler_create_task(u64 clone_flags, u64 func, u64 arg, long priority) {
  if (is_initialized == false) {
    return ERR_SYS_INVALID_OP;
  }
  /* Kernel threads only, EL0 tasks go through scheduler_create_user_task */
  if ((clone_flags & PF_KTHREAD) == 0) {
    return ERR_SYS_INVALID_OP;
  }

  preempt_disable();
  struct TaskBlock *p = (struct TaskBlock *)get_free_page();
  if (!p) {
    preempt_enable();
    return ERR_MEM_OUT_OF_MEMORY;
  }
  memzero((u64)p, TASK_SIZE);

  priority = clamp_priority(priority);
  p->state = TASK_RUNNING;
  p->priority = priority;
  p->counter = priority;
  p->preempt_count = 1;

  /* cpu_new_task consumes x19/x20 as the entry function and its argument */
  p->cpu_context.x19 = func;
  p->cpu_context.x20 = arg;
  p->cpu_context.sp = ((u64)p + TASK_SIZE) & ~15ULL;
  p->cpu_context.lr = get_cpu_new_task_addr();

  int slot = task_table_alloc_slot();
  if (slot < 0) {
    free_page((u64)p);
    preempt_enable();
    return ERR_GEN_NO_MEMORY;
  }
  task[slot] = p;
  num_tasks++;
  preempt_enable();
  return SUCCESS;
}

ErrorCode scheduler_create_user_task(u64 user_func) {
  if (is_initialized == false) {
    return ERR_SYS_INVALID_OP;
  }

  preempt_disable();

  struct TaskBlock *p = (struct TaskBlock *)get_free_page();
  if (!p) {
    preempt_enable();
    return ERR_MEM_OUT_OF_MEMORY;
  }
  memzero((u64)p, TASK_SIZE);

  u64 ustack = (u64)get_free_page();
  if (!ustack) {
    free_page((u64)p);
    preempt_enable();
    return ERR_MEM_OUT_OF_MEMORY;
  }

  /* Seed the exception frame at the top of the task page, cpu_new_task will eret into it */
  ProcessStateRegisters *regs = get_current_pstate(p);
  regs->pc = user_func;
  regs->pstate = PSR_MODE_EL0t;
  regs->sp = ustack + PAGE_SIZE;
  p->stack = ustack;

  /* x19 == 0 makes cpu_new_task take the ret_to_user path, sp must land on the frame */
  p->cpu_context.x19 = 0;
  p->cpu_context.lr = get_cpu_new_task_addr();
  p->cpu_context.sp = (u64)regs + 16;

  p->state = TASK_RUNNING;
  p->priority = DEFAULT_PRIORITY;
  p->counter = DEFAULT_PRIORITY;
  p->preempt_count = 1;

  int slot = task_table_alloc_slot();
  if (slot < 0) {
    free_page(ustack);
    free_page((u64)p);
    preempt_enable();
    return ERR_GEN_NO_MEMORY;
  }
  task[slot] = p;
  num_tasks++;
  preempt_enable();
  return SUCCESS;
}

ProcessStateRegisters *get_current_pstate(struct TaskBlock *task) {
  // We will save the ProcessStateRegisters at the top of the stack
  u64 p = (u64)task + TASK_SIZE - sizeof(ProcessStateRegisters);
  return (ProcessStateRegisters *)p;
}

void scheduler_exit_task() {
  preempt_disable();
  for (u8 i = 0; i < NUM_TASKS; i++) {
    if (task[i] == current) {
      current->state = TASK_ZOMBIE;
    }
  }
  if (current->stack) {
    free_page(current->stack);
  }
  preempt_enable();
  schedule();
}

/* Commit the current task to BLOCKED on chan and yield. IRQs are masked across the switch,
   mirroring scheduler_tick_handler, and restored to the caller's state once we're woken. */
static void block_current_on(void *chan, u64 deadline) {
  u64 flags = irq_save_flags();
  irq_disable();

  current->wait_channel = chan;
  current->wake_deadline = deadline;
  current->state = TASK_BLOCKED;
  current->counter = 0;

  _schedule(); /* switches away, returns here once we're marked runnable again */

  current->wait_channel = NULL;
  current->wake_deadline = 0;
  irq_restore_flags(flags);
}

void scheduler_block_on(void *chan) {
  block_current_on(chan, 0);
}

void scheduler_block_on_timeout(void *chan, u32 timeout_ms) {
  u64 deadline = timer_get_ticks() + (u64)timeout_ms * (CLOCK_HZ / 1000U);
  /* 0 is the "no timeout" sentinel, nudge a zero deadline forward so it still fires */
  if (deadline == 0) deadline = 1;
  block_current_on(chan, deadline);
}

void scheduler_wake_chan(void *chan) {
  u64 flags = irq_save_flags();
  irq_disable();

  for (int i = 0; i < NUM_TASKS; i++) {
    struct TaskBlock *t = task[i];
    if (t && t->state == TASK_BLOCKED && t->wait_channel == chan) {
      t->state = TASK_RUNNING;
      t->wake_deadline = 0;
      if (t->counter <= 0) t->counter = t->priority;
    }
  }

  irq_restore_flags(flags);
}
