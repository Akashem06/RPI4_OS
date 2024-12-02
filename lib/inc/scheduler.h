#pragma once

#define CPU_CONTEXT_OFFSET	    0 		// offset of cpu_context in TaskBlock 

#ifndef __ASSEMBLER__
#include "common.h"

#define THREAD_SIZE             4096L
#define NUM_TASKS               10L

#define TASK_RUNNING			0L
#define TASK_SLEEPING			1L
#define TASK_ZOMBIE				2L
#define TASK_BLOCKED			3L

#define PF_KTHREAD		        2UL

extern volatile struct TaskBlock *current;
extern volatile struct TaskBlock *task[NUM_TASKS];
extern volatile u8 num_tasks;

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

struct TaskBlock {
    struct CPUContext cpu_context;
    long state;
    long counter;
    long priority;
    long preempt_count;

	unsigned long stack;
	unsigned long flags;
} __attribute__((aligned(16)));


typedef struct {
	unsigned long regs[31];		// All registers X0-X30
	unsigned long sp;			// Stack pointer
	unsigned long pc;			// Program counteer
	unsigned long pstate;		// Processor state register (PSTATE) CONDITION FLAGS NZCV etc.
} ProcessStateRegisters;

#define MIN_PRIORITY     1
#define MAX_PRIORITY     10
#define DEFAULT_PRIORITY 5
#define MIN_TIMESLICE    2
#define STARVATION_LIMIT 100

extern void scheduler_init(void);
void schedule(void);
extern void scheduler_tick_handler(void);
void preempt_disable(void);
void preempt_enable(void);
void switch_to(struct TaskBlock* next);

extern u64 get_cpu_new_task_addr(void);
int scheduler_create_task(u64 func, u64 arg, long priority);
int move_task_to_user_mode(u64 func);
void scheduler_exit_task();
ProcessStateRegisters *get_current_pstate(struct TaskBlock *task);
void cpu_context_switch(struct TaskBlock* prev, struct TaskBlock* next);

#define INIT_TASK \
/* CpuContext */	{ {0,0,0,0,0,0,0,0,0,0,0,0,0}, \
/* state, counter, priority, preempt_count */	0, 0, 1, 0 \
}

#endif