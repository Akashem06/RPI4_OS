#include "common.h"
#include "mini_uart.h"

void kernal_main() {
    uart_init();
    uart_transmit_string("Aryan's bare-metal OS booting up. . . \n");

    #if RPI_VERSION == 3
    uart_transmit_string("\tBoard: Raspberry Pi 3\n")
    #elif RPI_VERSION == 4
    uart_transmit_string("\tBoard: Raspberry Pi 4\n")
    #endif

    uart_transmit_string("Done\n");

    while (1) {
        uart_transmit(uart_receive());
    }
}