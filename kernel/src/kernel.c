#include "kernel.h"
#include "log.h"
#include "mini_uart.h"
#include "utils.h"

// Simple delay function that doesn't rely on timer.h
void simple_delay(unsigned int count) {
    for (volatile unsigned int i = 0; i < count; i++) {
        // Just a busy wait
    }
}

void kernel_init() {
    // Initialize mini UART for output
    mini_uart_init();
    
    // Initialize logging with mini UART
    log_init(LOG_MODE_MINIUART);
    
    // Get execution level
    int el = get_el();
    
    // Print initial message with execution level
    log("QEMU Test: Kernel booted at EL%d\n\r", el);
}

void kernel_main() {
    // Initialize kernel
    kernel_init();
    
    // Print welcome message
    log("==============================\n\r");
    log("  RPI OS Running in QEMU!\n\r");
    log("==============================\n\r");
    
    int counter = 0;
    
    // Main loop - keep sending messages to verify QEMU is working
    while (1) {
        log("Counter: %d\n\r", counter++);
        
        if (counter % 5 == 0) {
            log("This is the %d-th iteration\n\r", counter);
        }
        
        // Simple delay without relying on timer.h
        simple_delay(50000000);
    }
}