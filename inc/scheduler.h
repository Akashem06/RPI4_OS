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
    struct CPUContext cpu_context __attribute__((aligned(16)));
    long state __attribute__((aligned(8)));     
    long counter __attribute__((aligned(8)));
    long priority __attribute__((aligned(8)));
    long preempt_count __attribute__((aligned(8)));
} __attribute__((aligned(16)));


#define MIN_PRIORITY      1
#define MAX_PRIORITY     10
#define DEFAULT_PRIORITY  5
#define MIN_TIMESLICE    2
#define STARVATION_LIMIT 100

extern void scheduler_init(void);
void schedule(void);
extern void scheduler_tick_handler(void);
void preempt_disable(void);
void preempt_enable(void);
void switch_to(struct TaskBlock* next);

extern u64 get_cpu_new_task_addr(void);
long scheduler_create_task(u64 func, u64 arg, long priority);
void cpu_context_switch(struct TaskBlock* prev, struct TaskBlock* next);

#define INIT_TASK \
/* CpuContext */	{ {0,0,0,0,0,0,0,0,0,0,0,0,0}, \
/* state, counter, priority, preempt_count */	0, 0, 1, 0 \
}

#endif