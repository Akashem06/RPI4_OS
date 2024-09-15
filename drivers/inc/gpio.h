#pragma once

#include "common.h"
#include "base.h"
#include "utils.h"

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

typedef struct {
    reg32 reserved;
    reg32 data[2];
} GpioPinData;

// Based off of data sheet
// https://datasheets.raspberrypi.com/bcm2711/bcm2711-peripherals.pdf
typedef struct {
    reg32 function_select[6];
    GpioPinData output_set;
    GpioPinData output_clear;
    GpioPinData level;
    GpioPinData event_detect;
    GpioPinData rising_edge;
    GpioPinData falling_edge;
    GpioPinData pin_high;
    GpioPinData pin_low;
    GpioPinData async_rising_edge;
    GpioPinData async_falling_edge;
    reg32 reserved;
    reg32 pupd_enable;
    reg32 pupd_enable_clocks[2];
} GpioRegisters;

void gpio_set_function(u8 pin_number, GpioFunctions func);
void gpio_enable(u8 pin_number);
void gpio_set_high(u8 pin_number);
void gpio_set_low(u8 pin_number);

#define GPIO_BASE (PBASE + 0x00200000) // GPIO register base address
#define GPIO_REGS (( volatile GpioRegisters * )(GPIO_BASE))
