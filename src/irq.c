#include "utils.h"
#include "log.h"
#include "entry.h"
#include "peripherals/irq.h"
#include "peripherals/aux_reg.h"
#include "peripherals/mini_uart.h"
#include "peripherals/uart.h"
#include "gpio.h"

const char entry_error_messages[16][32] = {
	"SYNC_INVALID_EL1t",
	"IRQ_INVALID_EL1t",		
	"FIQ_INVALID_EL1t",		
	"ERROR_INVALID_EL1T",		

	"SYNC_INVALID_EL1h",
	"IRQ_INVALID_EL1h",		
	"FIQ_INVALID_EL1h",		
	"ERROR_INVALID_EL1h",		

	"SYNC_INVALID_EL0_64",		
	"IRQ_INVALID_EL0_64",		
	"FIQ_INVALID_EL0_64",		
	"ERROR_INVALID_EL0_64",	

	"SYNC_INVALID_EL0_32",		
	"IRQ_INVALID_EL0_32",		
	"FIQ_INVALID_EL0_32",		
	"ERROR_INVALID_EL0_32"	
};

void show_invalid_entry_message(u32 type, u64 esr, u64 address) {
    log("ERROR CAUGHT: %s - %d\n", entry_error_messages[type], type);
}

void enable_interrupt_controller() {
    #if RPI_VERSION == 4
        IRQ_REGS->irq0_enable_0 = AUX_IRQ;
        IRQ_REGS->irq0_enable_1 = UART_IRQ;
    #endif

    #if RPI_VERSION == 3
        IRQ_REGS->irq0_enable_1 = AUX_IRQ;
    #endif
}

void handle_irq() {
    u32 irq_low;
    u32 irq_high;

#if RPI_VERSION == 4
    irq_low = IRQ_REGS->irq0_pending_0; // Interrupts 0-31
    irq_high = IRQ_REGS->irq0_pending_1; // Interrupts 32-63
#endif

#if RPI_VERSION == 3
    irq_low = IRQ_REGS->irq0_pending_1;
    irq_high = 0;
#endif

    while(irq_low || irq_high) {
        if (irq_low & AUX_IRQ) {
            irq_low &= ~AUX_IRQ;

            while((AUX_REGS->mu_iir & 4) == 4) {
                mini_uart_transmit(mini_uart_receive());
                mini_uart_transmit('\n');
            }
        }

        if (irq_high & UART_IRQ) {
            irq_high &= ~UART_IRQ;
            handle_uart_irq();
        }

        if (irq_low & SYS_TIMER_IRQ_1) {
            irq_low &= ~SYS_TIMER_IRQ_1;
            // handle_timer_1();
        }

        if (irq_low & SYS_TIMER_IRQ_3) {
            irq_low &= ~SYS_TIMER_IRQ_3;
            // handle_timer_3();
        }
    }

}
