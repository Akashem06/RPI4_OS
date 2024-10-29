#include "kernel.h"
#include "common.h"
#include "log.h"
#include "utils.h"
#include "irq.h"

#include <stdint.h>

UartSettings settings = {
    .uart = UART0,
    .tx = 14,
    .rx = 15,
};

char ch = 0U;
uint8_t flag = 0;

void handle_uart0_irq() {
    /* If RX interrupt. */
    flag = 1;
    if (settings.uart->mis & (1 << 4)) {
        flag = 2;
        settings.uart->icr |= (1 << 4);
        ch = settings.uart->dr & 0xFF;
    } 
    
    /* If TX interrupt. */
    if (settings.uart->mis & (1 << 5)) {
        settings.uart->icr |= (1 << 5);
    }
    /* If Overrun interrupt. */
    if (settings.uart->mis & (1 << 10)) {
        settings.uart->icr |= (1 << 10);
    }
}

void kernel_init() {
    uart_init(&settings);
    log_init(LOG_MODE_UART);
    int el = get_el();
    log("Hello! Welcome to UART IRQ sample app. EL: %d\n\r", el);

    irq_init_vectors();
    enable_interrupt_controller();
    irq_enable();
}

void kernel_main() {
    kernel_init();
    log("Kernel initialized. Enter characters into PuTTY monitor:\n\r");
    
    while (1) {
        if (flag) {
            log("Flag updated: %d\r\n", flag);
            flag = 0;
        }
        if (ch) {
            uart_transmit(ch);
            ch = 0U;
        }
    }
}
