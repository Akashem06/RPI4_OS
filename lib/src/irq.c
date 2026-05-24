#include "irq.h"

#include "arm_generic_timer.h"
#include "aux_reg.h"
#include "bcm2711_gic.h"
#include "bcm2711_periph_io.h"
#include "entry.h"
#include "gpio.h"
#include "log.h"
#include "mini_uart.h"
#include "scheduler.h"
#include "timer.h"
#include "uart.h"
#include "utils.h"

const char entry_error_messages[17][32] = {
  "SYNC_INVALID_EL1t",   "IRQ_INVALID_EL1t",   "FIQ_INVALID_EL1t",   "ERROR_INVALID_EL1T",

  "SYNC_INVALID_EL1h",   "IRQ_INVALID_EL1h",   "FIQ_INVALID_EL1h",   "ERROR_INVALID_EL1h",

  "SYNC_INVALID_EL0_64", "IRQ_INVALID_EL0_64", "FIQ_INVALID_EL0_64", "ERROR_INVALID_EL0_64",

  "SYNC_INVALID_EL0_32", "IRQ_INVALID_EL0_32", "FIQ_INVALID_EL0_32", "ERROR_INVALID_EL0_32",

  "SYSCALL_ERROR",
};

void show_invalid_entry_message(u32 type, u64 esr, u64 address, u64 fault_addr_reg, u64 stack_pointer) {
  log("ERROR CAUGHT: %s - %d. ESR: %d Address: %d\r\n", entry_error_messages[type], type, esr, address);
  for (int i = 0; i < CLOCK_HZ; i++) {
    __asm("NOP");
  }
  log("Fault addr_reg: %d, stack pointer: %d\r\n", fault_addr_reg, stack_pointer);
}

void print_register(u64 reg_val, u64 reg_num) {
  log("REG_NUMBER: %d, VALUE: %d\n\r", reg_num, reg_val);
  for (int i = 0; i < CLOCK_HZ; i++) {
    __asm("NOP");
  }
}

void enable_interrupt_controller() {
#if RPI_VERSION == 4
  REG_WR(IRQ_REGS->irq0_disable_0, 0xFFFFFFFF);
  REG_WR(IRQ_REGS->irq0_disable_1, 0xFFFFFFFF);
  REG_WR(IRQ_REGS->irq0_disable_2, 0xFFFFFFFF);

  // Enable basic GPU0 interrupts
  REG_WR(IRQ_REGS->irq0_enable_0, IRQ_TIMER_0 | IRQ_TIMER_1 | IRQ_TIMER_2 | IRQ_TIMER_3 | IRQ_CODEC_0 | IRQ_CODEC_1 |
                                    IRQ_CODEC_2 | IRQ_JPEG | IRQ_ISP | IRQ_USB | IRQ_3D | IRQ_DMA_0 | IRQ_AUX);

  // Enable GPU1 interrupts
  REG_WR(IRQ_REGS->irq0_enable_1, IRQ_I2C_SPI_SLV | IRQ_PWA0 | IRQ_PWA1 | IRQ_SMI | IRQ_GPIO_0 | IRQ_GPIO_1 | IRQ_GPIO_2 |
                                    IRQ_GPIO_3 | IRQ_I2C | IRQ_SPI | IRQ_PCM | IRQ_UART_0 | IRQ_UART_2 | IRQ_UART_3 |
                                    IRQ_UART_4 | IRQ_UART_5);
#elif RPI_VERSION == 3
  REG_WR(IRQ_REGS->irq0_enable_1, REG_RD(IRQ_REGS->irq0_enable_1) | AUX_IRQ);
#endif
}

void handle_irq() {
  u32 iar = gic_acknowledge();
  u32 intid = iar & 0x3FFU;

  // Spurious interrupt, nothing pending
  if (intid >= 1020U) {
    return;
  }

  if (intid == GENERIC_TIMER_PPI) {
    // Rearm and EOI before scheduling, a switch to a fresh task may not return here
    generic_timer_rearm();
    gic_end(iar);
    scheduler_tick_handler();
    return;
  }

  // Unhandled interrupt, acknowledge it so the GIC does not wedge
  gic_end(iar);
}
