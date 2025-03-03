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

/**
 * @brief   Initialize the task scheduler
 */
extern void scheduler_init(void);

/**
 * @brief
 */
void schedule(void);
extern void scheduler_tick_handler(void);
void preempt_disable(void);
void preempt_enable(void);
void switch_to(struct TaskBlock *next);

extern u64 get_cpu_new_task_addr(void);
int scheduler_create_task(u64 clone_flags, u64 func, u64 arg, long priority);
int move_task_to_user_mode(u64 func);
void scheduler_exit_task();
ProcessStateRegisters *get_current_pstate(struct TaskBlock *task);
void cpu_context_switch(struct TaskBlock *prev, struct TaskBlock *next);

#define INIT_TASK /* CpuContext */             \
  { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, \
    0, /* state */                             \
    0, /* counter */                           \
    1, /* priority */                          \
    0, /* preempt_count */                     \
    0, /* stack */                             \
    0 /* flags */ }

#endif

/** @} */
