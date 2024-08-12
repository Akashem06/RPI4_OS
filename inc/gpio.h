#pragma once

#include "peripherals/gpio.h"
#include "utils.h"
#include "common.h"

typedef enum {
    GF_INPUT,
    GF_OUTPUT,
    GF_ALT0 = 4,
    GF_ALT1,
    GF_ALT2,
    GF_ALT3,
    GF_ALT4 = 3,
    GF_ALT5 = 2
} GpioFunctions;

void gpio_set_func(u8 pin_number, GpioFunctions func);
void gpio_enable(u8 pin_number);
