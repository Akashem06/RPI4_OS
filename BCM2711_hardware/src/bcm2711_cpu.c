/*******************************************************************************************************************************
 * @file   bcm2711_cpu.c
 *
 * @brief  BCM2711 CPU abstraction
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "bcm2711_cpu.h"

extern u32 get_cpu_id(void) {
  u64 mpidr;
  asm volatile("mrs %0, mpidr_el1" : "=r"(mpidr));

  /* Bottom 8 bits contain the CPU Id as per ARM specification */
  return mpidr & 0xFFU;
}

extern void cpu_yield(void) {
  asm volatile("wfe");
}

extern void wakeup_cpu(void) {
  asm volatile("sev");
}

extern void cpu_wait_for_interrupt(void) {
  asm volatile("wfi");
}

extern void cpu_enable_irq(void) {
  asm volatile("msr daifclr, #2");
}

extern void cpu_disable_irq(void) {
  asm volatile("msr daifset, #2");
}
