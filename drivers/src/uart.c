#include "uart.h"
#include "gpio.h"

UartSettings *config;

char uart_output_queue[UART_MAX_QUEUE];
u16 uart_output_queue_write = 0;
u16 uart_output_queue_read = 0;

bool uart_read_ready()  {
    return !(config->uart->fr & (1 << 4));
}

void uart_transmit(char c) {
    while (config->uart->fr & (1 << 5)); // Wait until TX FIFO is not full
    config->uart->dr = c;
}

char uart_receive() {
    while (config->uart->fr & (1 << 4)); // Wait until RX FIFO is not empty
    return config->uart->dr & 0xFF;
}

void uart_transmit_string(char *str) {
    while (*str) {
        if (*str == '\n') {
            uart_transmit('\r');
        }

        uart_transmit(*str); // Send char at current address
        str++; // Increment memory address, going to next char
    }
}

void uart_init(UartSettings *settings) {
    config = settings;
    if (config->uart == UART0) {
        // TODO: Add functionality for pins 36/37. Right now they initalized wrong
        gpio_set_function(config->tx, GF_ALT0);
        gpio_set_function(config->rx, GF_ALT0);
    } else if (config->uart == UART2 || config->uart == UART3 || config->uart == UART4 || config->uart == UART5) {
        gpio_set_function(config->tx, GF_ALT4);
        gpio_set_function(config->rx, GF_ALT4);
    }
    gpio_enable(config->tx);
    gpio_enable(config->rx);

    config->uart->cr = 0; // Disable UART during setup

    // UART Reference Clock Freq = 48MHz
    // baud divisor = UARTCLK / (16 * baud_rate) 
	//               = 48 * 10^6 / (16 * 115200) = 26.0416666667
    //  integer part = 26
	//  fractional part = (int) ((0.0416666667 * 64) + 0.5) = roughly 3
    // TODO: Add custom calculation. Will likely use an init struct + figure out FPU :(
    config->uart->ibrd = 26;
    config->uart->fbrd = 3;

    config->uart->lcrh = (1 << 4) | (1 << 5) | (1 << 6); // 8 bits, no parity, 1 stop bit
    config->uart->imsc = (1 << 4); // Sets RX ISR
    config->uart->cr = (1 << 0) | (1 << 8) | (1 << 9); // Enable UART, TX, and RX. TODO: Add more functions? Loopback?
}
