#include "scheduler.h"

#include <stdbool.h>
#include <stddef.h>

#include "entry.h"
#include "irq.h"
#include "log.h"
#include "mem.h"
#include "mm.h"
#include "sysregs.h"
#include "timer.h"

struct TaskBlock init_task = INIT_TASK;
static bool is_initialized = false;

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
    if ((next->cpu_context.sp & 15) != 0) {
      log("ERROR: Stack pointer not 16-byte aligned\n");
      return;
    }

    if ((next->cpu_context.lr & 3) != 0) {
      log("ERROR: Link register not 4-byte aligned\n");
      return;
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

void _schedule(void) {
  preempt_disable();
  int next;

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

void scheduler_tick_handler() {
  if (!current) {
    return;
  }
  volatile u64 flags = 0;
  irq_save_flags(flags);
  if (--current->counter > 0 || current->preempt_count > 0) {
    irq_restore_flags(flags);
    return;
  }
  irq_disable();

  current->counter = 0;
  _schedule();
  irq_restore_flags(flags);
  irq_enable();
}

void scheduler_init() {
  current = (struct TaskBlock *)get_free_page();
  current = &init_task;
  task[0] = current;

  num_tasks = 1;

  timer_init(3, (CLOCK_HZ / 10), scheduler_tick_handler);

  is_initialized = true;
}

int scheduler_create_task(u64 clone_flags, u64 func, u64 arg, long priority) {
  if (is_initialized == false) {
    return 1;
  }
  if (num_tasks >= NUM_TASKS) {
    return 2;
  }

  preempt_disable();
  struct TaskBlock *p;

  p = (struct TaskBlock *)get_free_page();
  if (!p) return 3;

  if ((unsigned long)p < LOW_MEMORY || (unsigned long)p >= HIGH_MEMORY) {
    preempt_enable();
    return 4;
  }

  ProcessStateRegisters *childregs = get_current_pstate(p);
  memzero((u64)childregs, sizeof(ProcessStateRegisters));
  memzero((u64)p, sizeof(struct TaskBlock));

  if (clone_flags & PF_KTHREAD) {
    p->cpu_context.x19 = func;
    p->cpu_context.x20 = arg;
  } else {
    ProcessStateRegisters *cur_regs = get_current_pstate(current);
    *childregs = *cur_regs;
    childregs->regs[0] = 0;
    // childregs->sp = stack + PAGE_SIZE;
    // p->stack = stack;
  }

  p->state = TASK_RUNNING;
  p->priority = priority;
  p->counter = priority;
  p->preempt_count = 1;

  // Get the actual runtime address of cpu_new_task
  u64 new_task_addr = get_cpu_new_task_addr();

  p->cpu_context.x19 = func;
  p->cpu_context.x20 = arg;
  p->cpu_context.sp = ((u64)p + THREAD_SIZE) & ~15ULL;
  p->cpu_context.lr = new_task_addr;

  u8 pid = num_tasks++;
  task[pid] = p;
  preempt_enable();
  return 0;
}

int move_task_to_user_mode(u64 func) {
  ProcessStateRegisters *regs = get_current_pstate(current);
  memzero((u64)regs, sizeof(*regs));
  // Points to the function that needs to be executed next in user mode.
  // Kernel_exit will copy PC to the ELR_EL1 register, ensuring that we return to this function
  regs->pc = func;

  // This is copied to spsr_el1 by kernel_exit and becomes the new state of the processor after
  // leaving.
  regs->pstate = PSR_MODE_EL0t;

  // New user stack
  u64 stack = (u64)get_free_page();
  if (!stack) {
    return -1;
  }

  regs->sp = stack + PAGE_SIZE;
  current->stack = stack;
  return 0;
}

ProcessStateRegisters *get_current_pstate(struct TaskBlock *task) {
  // We will save the ProcessStateRegisters at the top of the stack
  u64 p = (u64)task + THREAD_SIZE - sizeof(ProcessStateRegisters);
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
