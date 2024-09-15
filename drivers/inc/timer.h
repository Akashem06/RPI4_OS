#pragma once

#include "base.h"
#include "common.h"

#define CLOCK_HZ 1000000
#define NUM_TIMERS 4

typedef struct {
    reg32 control_status;
    reg32 counter_lo;
    reg32 counter_hi;
    reg32 compare[4];
} TimerRegisters;

typedef struct {
    u32 interval;
    u32 cur_val;
    void (*handler)(void);
} Timer;

void timer_init(u8 timer_id, u32 interval, void (*handler)(void));
void handle_timer_irq();
u64 timer_get_ticks();
void timer_sleep();

#define TIMER_BASE (PBASE + 0x00003000) // Timer register base address
#define TIMER_REGS (( volatile TimerRegisters * )(TIMER_BASE))