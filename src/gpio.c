#include "gpio.h"

void gpio_set_func(u8 pin_number, GpioFunctions func) {
    u8 bit_start = (pin_number * 3) % 30;
    u8 reg = pin_number / 10;
    u32 selector = GPIO_REGS->function_select[reg];

    selector &= ~( 7 << bit_start );
    selector |= ( func << bit_start );

    GPIO_REGS->function_select[reg] = selector;
    
}

// Page 101 of BCM2835 ARM Peripheral datasheet
void gpio_enable(u8 pin_number) {
    GPIO_REGS->pupd_enable = 0;
    delay(150);
    GPIO_REGS->pupd_enable_clocks[ pin_number / 32 ] = 1 << (pin_number % 32);
    delay(150);
    GPIO_REGS->pupd_enable = 0;
    GPIO_REGS->pupd_enable_clocks[ pin_number / 32 ] = 0;
}