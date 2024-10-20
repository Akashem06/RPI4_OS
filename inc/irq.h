#pragma once

#include "base.h"
#include "common.h"

enum videocore_irqs_low {
    SYS_TIMER_IRQ_0 = 1,
    SYS_TIMER_IRQ_1 = 2,
    SYS_TIMER_IRQ_2 = 4,
    SYS_TIMER_IRQ_3 = 8,
    AUX_IRQ = (1 << 29),
};

enum videocore_irqs_high {
    UART_IRQ = (1 << (57 - 32) ),
};

struct arm_irq_regs_rpi4 {
    reg32 irq0_pending_0;
    reg32 irq0_pending_1;
    reg32 irq0_pending_2;
    reg32 res0;
    reg32 irq0_enable_0;
    reg32 irq0_enable_1;
    reg32 irq0_enable_2;
    reg32 res1;
    reg32 irq0_disable_0;
    reg32 irq0_disable_1;
    reg32 irq0_disable_2;
};

struct arm_irq_regs_rpi3 {
    reg32 irq0_pending_0;
    reg32 irq0_pending_1;
    reg32 irq0_pending_2;
    reg32 fiq_control;
    reg32 irq0_enable_1;
    reg32 irq0_enable_2;
    reg32 irq0_enable_0;
    reg32 res;
    reg32 irq0_disable_1;
    reg32 irq0_disable_2;
    reg32 irq0_disable_0;
};


#if RPI_VERSION == 3
    typedef struct arm_irq_regs_rpi3 IRQRegisters;
#endif

#if RPI_VERSION == 4
    typedef struct arm_irq_regs_rpi4 IRQRegisters;
#endif

#define IRQ_REGS ((IRQRegisters *)(PBASE + 0x0000B200))

void irq_init_vectors();
void irq_enable();
void irq_disable();
void irq_save_flags(u64 flags);
void irq_restore_flags(u64 flags);
void enable_interrupt_controller();
