/*******************************************************************************************************************************
 * @file   arm_generic_timer.c
 *
 * @brief  ARMv8 generic timer (CNTP) used as the scheduler preemption tick
 *
 * @date   2026-05-23
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>

/* Inter-component Headers */
#include "bcm2711_gic.h"
#include "common.h"
#include "irq_chip.h"
#include "scheduler.h"

/* Intra-component Headers */
#include "arm_generic_timer.h"

static u32 timer_interval = 0U; /* Counter ticks between scheduler ticks */

void generic_timer_init(u32 hz) {
  u64 freq;
  asm volatile("mrs %0, cntfrq_el0" : "=r"(freq));

  timer_interval = (u32)(freq / hz);

  /* Arm the down-counter and enable, CNTP_CTL bit0 = enable, bit1 = mask */
  asm volatile("msr cntp_tval_el0, %0" ::"r"((u64)timer_interval));
  asm volatile("msr cntp_ctl_el0, %0" ::"r"((u64)1));
  asm volatile("isb");
}

void generic_timer_rearm(void) {
  asm volatile("msr cntp_tval_el0, %0" ::"r"((u64)timer_interval));
}

/* Registered IRQ handler, reloads the timer then drives the scheduler tick */
static void timer_tick_isr(u32 intid, void *ctx) {
  (void)intid;
  (void)ctx;
  generic_timer_rearm();
  scheduler_tick_handler();
}

/* SchedTickSource::start, wires the timer PPI into the dispatcher and starts ticking */
static ErrorCode generic_timer_start(u32 hz) {
  ErrorCode err = irq_register_handler(GENERIC_TIMER_PPI, timer_tick_isr, NULL);
  if (err != SUCCESS) {
    return err;
  }
  irq_enable_line(GENERIC_TIMER_PPI, 0);
  generic_timer_init(hz);
  return SUCCESS;
}

static const struct SchedTickSource bcm2711_tick_source = {
  .start = generic_timer_start,
};

void bcm2711_tick_source_register(void) {
  scheduler_set_tick_source(&bcm2711_tick_source);
}
