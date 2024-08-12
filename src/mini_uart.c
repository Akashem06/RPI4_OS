#include "peripherals/mini_uart.h"
#include "gpio.h"

void uart_transmit(char c) {
    while(!(AUX_REGS->mu_lsr & 0x20)); // Checks if transmitter empty 
    
    AUX_REGS->mu_io = c;
}

void uart_transmit_string(char *message) {
    while (*message) {
        if (*message == '\n') {
            uart_transmit('\r');
        }

        uart_transmit(*message); // Send char at current address
        message++; // Increment memory address, going to next char
    }
}

char uart_receive() {
    while(!(AUX_REGS->mu_lsr & 0x1)); // Checks if transmitter empty 
    
    return AUX_REGS->mu_io & 0xFF;
}

void uart_init() {
    gpio_set_function(TX_PIN, GF_ALT5);
    gpio_set_function(RX_PIN, GF_ALT5);

    gpio_enable(TX_PIN);
    gpio_enable(RX_PIN);

    AUX_REGS->irq_enables = 1; // Enables ISR for all peripherals
    AUX_REGS->mu_control = 0; // Disables UART transmitter and receiver
    AUX_REGS->mu_ier = 0; // Disables ISR's for Mini UART
    AUX_REGS->mu_lcr = 3; // Sets UART data format to 8bit messages
    AUX_REGS->mu_mcr = 0; // Disables features like RTS (Request to Send) and CTS (Clear to Send)

    #if RPI_VERSION == 3
    // Defined in datasheet: System_Clock_Freq / ( 8 * (1 + mu_baudrate ) ) @ 250 MHz
    AUX_REGS->mu_baudrate = 270;
    #elif RPI_VERSION == 4
    // Defined in datasheet: System_Clock_Freq / ( 8 * (1 + mu_baudrate ) ) @ 500 MHz
    AUX_REGS->mu_baudrate = 541;
    #endif

    AUX_REGS->mu_control = 3; // Enables transmitter and receiver for UAR

    uart_transmit('\r');
    uart_transmit('\r');
    uart_transmit('\n');
}
