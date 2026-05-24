#include "timer.h"

#include "bcm2711_periph_io.h"
#include "irq.h"
#include "log.h"

Timer timers[NUM_TIMERS];

void timer_init(u8 timer_id, u32 interval, void (*handler)(void)) {
  if (timer_id < NUM_TIMERS) {
    timers[timer_id].interval = interval;
    timers[timer_id].cur_val = REG_RD(TIMER_REGS->counter_lo) + interval;
    timers[timer_id].handler = handler;
    REG_WR(TIMER_REGS->compare[timer_id], timers[timer_id].cur_val);
#if RPI_VERSION == 4
    REG_WR(IRQ_REGS->irq0_enable_0, REG_RD(IRQ_REGS->irq0_enable_0) | (1 << timer_id));
#endif

#if RPI_VERSION == 3
    REG_WR(IRQ_REGS->irq0_enable_1, REG_RD(IRQ_REGS->irq0_enable_1) | (1 << timer_id));
#endif
  }
}

void handle_timer_irq() {
  for (int i = 0; i < NUM_TIMERS; i++) {
    if (REG_RD(TIMER_REGS->control_status) & (1 << i)) {
      timers[i].cur_val += timers[i].interval;
      REG_WR(TIMER_REGS->compare[i], timers[i].cur_val);
      REG_WR(TIMER_REGS->control_status, (1 << i));  // Write 1 to clear the match flag

      if (timers[i].handler) {
        timers[i].handler();
      }
    }
  }
}

u64 timer_get_ticks() {
  u32 hi = REG_RD(TIMER_REGS->counter_hi);
  u32 lo = REG_RD(TIMER_REGS->counter_lo);

  // Double check hi value didn't change after setting it
  if (hi != REG_RD(TIMER_REGS->counter_hi)) {
    hi = REG_RD(TIMER_REGS->counter_hi);
    lo = REG_RD(TIMER_REGS->counter_lo);
  }

  return ((u64)hi << 32) | lo;
}

void timer_sleep(u32 ms) {
  u64 start = timer_get_ticks();

  while (timer_get_ticks() < start + (ms * 1000)) {
  }
}
