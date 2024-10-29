#pragma once

#include "base.h"
#include "common.h"

enum irq_sources {
    // GPU IRQs (0-31)
    IRQ_TIMER_0      = (1 << 0),
    IRQ_TIMER_1      = (1 << 1),
    IRQ_TIMER_2      = (1 << 2),
    IRQ_TIMER_3      = (1 << 3),
    IRQ_CODEC_0      = (1 << 4),
    IRQ_CODEC_1      = (1 << 5),
    IRQ_CODEC_2      = (1 << 6),
    IRQ_JPEG         = (1 << 7),
    IRQ_ISP          = (1 << 8),
    IRQ_USB          = (1 << 9),
    IRQ_3D           = (1 << 10),
    IRQ_DMA_0        = (1 << 16),
    IRQ_AUX          = (1 << 29),
    
    // GPU1 IRQs (32-63)
    IRQ_I2C_SPI_SLV  = (1 << (37-32)),
    IRQ_PWA0         = (1 << (45-32)),
    IRQ_PWA1         = (1 << (46-32)),
    IRQ_SMI          = (1 << (48-32)),
    IRQ_GPIO_0       = (1 << (49-32)),
    IRQ_GPIO_1       = (1 << (50-32)),
    IRQ_GPIO_2       = (1 << (51-32)),
    IRQ_GPIO_3       = (1 << (52-32)),
    IRQ_I2C          = (1 << (53-32)),
    IRQ_SPI          = (1 << (54-32)),
    IRQ_PCM          = (1 << (55-32)),
    IRQ_UART_0       = (1 << (57-32)),
    IRQ_UART_2       = (1 << (58-32)),
    IRQ_UART_3       = (1 << (59-32)),
    IRQ_UART_4       = (1 << (60-32)),
    IRQ_UART_5       = (1 << (61-32)),
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
