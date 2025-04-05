#include "irq.h"

#include "aux_reg.h"
#include "entry.h"
#include "gpio.h"
#include "log.h"
#include "mini_uart.h"
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
  IRQ_REGS->irq0_disable_0 = 0xFFFFFFFF;
  IRQ_REGS->irq0_disable_1 = 0xFFFFFFFF;
  IRQ_REGS->irq0_disable_2 = 0xFFFFFFFF;

  // Enable basic GPU0 interrupts
  IRQ_REGS->irq0_enable_0 = IRQ_TIMER_0 | IRQ_TIMER_1 | IRQ_TIMER_2 | IRQ_TIMER_3 | IRQ_CODEC_0 | IRQ_CODEC_1 | IRQ_CODEC_2 |
                            IRQ_JPEG | IRQ_ISP | IRQ_USB | IRQ_3D | IRQ_DMA_0 | IRQ_AUX;

  // Enable GPU1 interrupts
  IRQ_REGS->irq0_enable_1 = IRQ_I2C_SPI_SLV | IRQ_PWA0 | IRQ_PWA1 | IRQ_SMI | IRQ_GPIO_0 | IRQ_GPIO_1 | IRQ_GPIO_2 |
                            IRQ_GPIO_3 | IRQ_I2C | IRQ_SPI | IRQ_PCM | IRQ_UART_0 | IRQ_UART_2 | IRQ_UART_3 | IRQ_UART_4 |
                            IRQ_UART_5;
#elif RPI_VERSION == 3
  IRQ_REGS->irq0_enable_1 |= AUX_IRQ;
#endif
}

void handle_irq() {
  u32 irq_pending_0;
  u32 irq_pending_1;

#if RPI_VERSION == 4
  irq_pending_0 = IRQ_REGS->irq0_pending_0;  // Interrupts 0-31
  irq_pending_1 = IRQ_REGS->irq0_pending_1;  // Interrupts 32-63
#elif RPI_VERSION == 3
  irq_pending_0 = 0;
  irq_pending_1 = IRQ_REGS->irq0_pending_1;
#endif

  while (irq_pending_0 || irq_pending_1) {
    if (irq_pending_0 & IRQ_TIMER_0) {
      irq_pending_0 &= ~IRQ_TIMER_0;
      handle_timer_irq();
    }
    if (irq_pending_0 & IRQ_TIMER_1) {
      irq_pending_0 &= ~IRQ_TIMER_1;
      handle_timer_irq();
    }
    if (irq_pending_0 & IRQ_TIMER_2) {
      irq_pending_0 &= ~IRQ_TIMER_2;
      handle_timer_irq();
    }
    if (irq_pending_0 & IRQ_TIMER_3) {
      irq_pending_0 &= ~IRQ_TIMER_3;
      handle_timer_irq();
    }

    // Handle AUX interrupts (including mini UART)
    if (irq_pending_0 & IRQ_AUX) {
      irq_pending_0 &= ~IRQ_AUX;

      u32 aux_irq = AUX_REGS->irq_status;

      // Mini UART
      // if ((AUX_REGS->mu_iir & 4) == 4) {
      //     while((AUX_REGS->mu_iir & 4) == 4) {
      //         mini_uart_transmit(mini_uart_receive());
      //         mini_uart_transmit('\n');
      //     }
      // }

      // SPI 1
      if (aux_irq & 2) {
        // Handle SPI1 interrupt
      }

      // SPI 2
      if (aux_irq & 4) {
        // Handle SPI2 interrupt
      }
    }

    // Handle UART0 interrupt
    if (irq_pending_1 & IRQ_UART_0) {
      irq_pending_1 &= ~IRQ_UART_0;
      handle_uart0_irq();
    }

    if (irq_pending_1 & IRQ_UART_2) {
      irq_pending_1 &= ~IRQ_UART_2;
    }

    if (irq_pending_1 & IRQ_UART_3) {
      irq_pending_1 &= ~IRQ_UART_3;
    }

    if (irq_pending_1 & IRQ_UART_4) {
      irq_pending_1 &= ~IRQ_UART_4;
    }

    if (irq_pending_1 & IRQ_UART_5) {
      irq_pending_1 &= ~IRQ_UART_5;
    }

    // Handle GPIO interrupts
    if (irq_pending_1 & IRQ_GPIO_0) {
      irq_pending_1 &= ~IRQ_GPIO_0;
      // Add GPIO interrupt handler
    }
    if (irq_pending_1 & IRQ_GPIO_1) {
      irq_pending_1 &= ~IRQ_GPIO_1;
      // Add GPIO interrupt handler
    }
    if (irq_pending_1 & IRQ_GPIO_2) {
      irq_pending_1 &= ~IRQ_GPIO_2;
      // Add GPIO interrupt handler
    }
    if (irq_pending_1 & IRQ_GPIO_3) {
      irq_pending_1 &= ~IRQ_GPIO_3;
      // Add GPIO interrupt handler
    }

    // Handle I2C interrupt
    if (irq_pending_1 & IRQ_I2C) {
      irq_pending_1 &= ~IRQ_I2C;
      // Add I2C interrupt handler
    }

    // Handle SPI interrupt
    if (irq_pending_1 & IRQ_SPI) {
      irq_pending_1 &= ~IRQ_SPI;
      // Add SPI interrupt handler
    }

    // Handle PCM/I2S interrupt
    if (irq_pending_1 & IRQ_PCM) {
      irq_pending_1 &= ~IRQ_PCM;
      // Add PCM interrupt handler
    }
  }
}
