#include "gpio.h"

#include "bcm2711_periph_io.h"

void gpio_set_function(u8 pin_number, GpioFunctions func) {
  u8 bit_start = (pin_number * 3) % 30;
  u8 reg = pin_number / 10;
  u32 selector = REG_RD(GPIO_REGS->function_select[reg]);

  selector &= ~(7 << bit_start);
  selector |= (func << bit_start);

  REG_WR(GPIO_REGS->function_select[reg], selector);
}

// Page 101 of BCM2835 ARM Peripheral datasheet
void gpio_enable(u8 pin_number) {
  u8 bank = pin_number / 32;
  u32 mask = 1 << (pin_number % 32);

  REG_WR(GPIO_REGS->pupd_enable, 0);
  delay(150);
  REG_WR(GPIO_REGS->pupd_enable_clocks[bank], REG_RD(GPIO_REGS->pupd_enable_clocks[bank]) | mask);
  delay(150);
  REG_WR(GPIO_REGS->pupd_enable, 0);
  REG_WR(GPIO_REGS->pupd_enable_clocks[bank], REG_RD(GPIO_REGS->pupd_enable_clocks[bank]) & ~mask);
}

void gpio_set_high(u8 pin_number) {
  if (pin_number < 32) {
    REG_WR(GPIO_REGS->output_set.data[0], 1 << pin_number);  // Write 1 to set
  } else {
    REG_WR(GPIO_REGS->output_set.data[1], 1 << (pin_number - 32));
  }
}

void gpio_set_low(u8 pin_number) {
  if (pin_number < 32) {
    REG_WR(GPIO_REGS->output_clear.data[0], 1 << pin_number);  // Write 1 to clear
  } else {
    REG_WR(GPIO_REGS->output_clear.data[1], 1 << (pin_number - 32));
  }
}
