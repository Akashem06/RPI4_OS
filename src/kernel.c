#include "common.h"
#include "gpio.h"
#include "log.h"
#include "utils.h"
#include "irq.h"
#include "peripherals/uart.h"
#include "peripherals/irq.h"

UartSettings settings = {
    .uart = UART0,
    .tx = 14,
    .rx = 15,
};

void kernel_main() {
    uart_init(&settings);
    gpio_set_function(17, GF_OUTPUT);

    irq_init_vectors();
    enable_interrupt_controller();
    irq_enable();

    // int el = get_el();

    while (1) {
        gpio_set_high(17);
        delay(1000000);
        gpio_set_low(17);
        delay(1000000);
        // mini_uart_transmit(mini_uart_receive());
        // log("Hello World! %d\r\n", el);
    }
}