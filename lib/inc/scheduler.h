#pragma once

/*******************************************************************************************************************************
 * @file   scheduler.h
 *
 * @brief  Main scheduler header file
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup Scheduler OS Scheduler Library
 * @brief    Library that supports CFS, Priority/regular round-robin, EDF, First-come-first-serve
 * scheduling algorithms
 * @{
 */

#define CPU_CONTEXT_OFFSET 0  // offset of cpu_context in TaskBlock

#ifndef __ASSEMBLER__
#include "common.h"
#include "error.h"

/** @brief  Size allocated for Task */
#define TASK_SIZE 4096

#define NUM_TASKS 10

#define TASK_RUNNING 0L
#define TASK_SLEEPING 1L
#define TASK_ZOMBIE 2L
#define TASK_BLOCKED 3L

#define PF_KTHREAD 2UL

extern struct TaskBlock *current;
extern struct TaskBlock *task[NUM_TASKS];
extern u8 num_tasks;

/**
 * @brief   CPU register context to be stored and loaded during context switches
 */
struct CPUContext {
  u64 x19;
  u64 x20;
  u64 x21;
  u64 x22;
  u64 x23;
  u64 x24;
  u64 x25;
  u64 x26;
  u64 x27;
  u64 x28;
  u64 fp;
  u64 sp;
  u64 lr;
};

/**
 * @brief   Task Storage struct to track CPU registers, state and process information
 */
struct TaskBlock {
  struct CPUContext cpu_context;
  long state;
  long counter;
  long priority;
  long preempt_count;

  unsigned long stack;
  unsigned long flags;

  void *wait_channel;  /* non-NULL while blocked on a channel (see scheduler_block_on) */
  u64 wake_deadline;   /* timer tick to auto-wake a timed block, 0 = no timeout */
};

typedef struct {
  unsigned long regs[31];  // All registers X0-X30
  unsigned long sp;        // Stack pointer
  unsigned long pc;        // Program counteer
  unsigned long pstate;    // Processor state register (PSTATE) CONDITION FLAGS NZCV etc.
} ProcessStateRegisters;

#define MIN_PRIORITY 1
#define MAX_PRIORITY 10
#define DEFAULT_PRIORITY 5
#define MIN_TIMESLICE 2
#define STARVATION_LIMIT 100

/** @brief  Default preemption tick frequency (Hz) */
#define SCHED_TICK_HZ 100

/**
 * @brief   Periodic tick source, supplied by the board (e.g. the ARM generic timer)
 */
struct SchedTickSource {
  ErrorCode (*start)(u32 hz); /**< Begin delivering scheduler ticks at @p hz */
};

/**
 * @brief   Initialize the task scheduler (task table only, no hardware)
 */
extern void scheduler_init(void);

/**
 * @brief   Register the board's periodic tick source for preemption
 * @param   src Ops table whose start() enables the tick, must outlive use
 */
void scheduler_set_tick_source(const struct SchedTickSource *src);

/**
 * @brief   Start preemption ticks through the registered tick source
 * @param   hz Ticks per second
 * @return  SUCCESS, or an error if no tick source has been registered
 */
ErrorCode scheduler_start_tick(u32 hz);

/**
 * @brief
 */
void schedule(void);
extern void scheduler_tick_handler(void);

/**
 * @brief   Block the current task on a wait channel until woken
 * @details Sets the task BLOCKED with @p chan as its wait channel and yields the CPU.
 *          Returns once scheduler_wake_chan(chan) marks it runnable again. The caller is
 *          responsible for closing the lost-wakeup window (test its condition and commit to
 *          blocking under a spinlock/IRQ guard, then re-test after this returns).
 * @param   chan Opaque channel address, usually the object being waited on
 */
void scheduler_block_on(void *chan);

/**
 * @brief   Block the current task on a channel with a timeout
 * @details Like scheduler_block_on, but the scheduler tick also wakes the task once
 *          @p timeout_ms has elapsed even if nobody signals the channel. The caller re-tests
 *          its condition on return to tell a real wake from a timeout.
 * @param   chan       Opaque channel address
 * @param   timeout_ms Milliseconds after which the task is force-woken
 */
void scheduler_block_on_timeout(void *chan, u32 timeout_ms);

/**
 * @brief   Wake every task currently blocked on a channel
 * @details IRQ-safe, so it may be called from a worker thread or an ISR. Marks matching tasks
 *          runnable, the woken tasks re-test their condition themselves.
 * @param   chan Channel to wake
 */
void scheduler_wake_chan(void *chan);
void preempt_disable(void);
void preempt_enable(void);
void switch_to(struct TaskBlock *next);

extern u64 get_cpu_new_task_addr(void);
ErrorCode scheduler_create_task(u64 clone_flags, u64 func, u64 arg, long priority);

/**
 * @brief   Spawn a task that begins executing at EL0 (user mode)
 * @details Seeds a fresh task's exception frame so that when it is first
 *          scheduled, cpu_new_task takes the ret_to_user path and erets into
 *          @p user_func at EL0 with its own user stack. The program reaches the
 *          kernel only through the svc syscall interface.
 * @param   user_func  Address of the EL0 entry function (must exit via call_sys_exit)
 * @return  SUCCESS on success, error code otherwise
 */
ErrorCode scheduler_create_user_task(u64 user_func);

void scheduler_exit_task();
ProcessStateRegisters *get_current_pstate(struct TaskBlock *task);
void cpu_context_switch(struct TaskBlock *prev, struct TaskBlock *next);

#define INIT_TASK /* CpuContext */                                    \
  {                                                                   \
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0, /* state */         \
        0,                                        /* counter */       \
        1,                                        /* priority */      \
        0,                                        /* preempt_count */ \
        0,                                        /* stack */         \
        0,                                        /* flags */         \
        0,                                        /* wait_channel */   \
        0                                         /* wake_deadline */ \
  }

#endif

/** @} */
