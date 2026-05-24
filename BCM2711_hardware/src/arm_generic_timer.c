/*******************************************************************************************************************************
 * @file   arm_generic_timer.c
 *
 * @brief  ARMv8 generic timer (CNTP) used as the scheduler preemption tick
 *
 * @date   2026-05-23
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "common.h"

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
