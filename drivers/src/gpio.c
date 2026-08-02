/*******************************************************************************************************************************
 * @file   gpio.c
 *
 * @brief  GPIO driver for the BCM2711 SoC
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "bcm2711_periph_io.h"
#include "device.h"

/* Intra-component Headers */
#include "gpio.h"

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

/* Device wrapper, exposes the GPIO block through the unified device interface */

static ErrorCode gpio_dev_ioctl(struct Device *dev, u32 cmd, u64 arg) {
  (void)dev;
  u8 pin = arg & 0xFF;

  switch (cmd) {
    case GPIO_IOCTL_SET_FUNCTION:
      gpio_set_function(pin, (GpioFunctions)((arg >> 8) & 0xFF));
      return SUCCESS;
    case GPIO_IOCTL_ENABLE:
      gpio_enable(pin);
      return SUCCESS;
    case GPIO_IOCTL_SET_HIGH:
      gpio_set_high(pin);
      return SUCCESS;
    case GPIO_IOCTL_SET_LOW:
      gpio_set_low(pin);
      return SUCCESS;
    default:
      return ERR_SYS_NOT_SUPPORTED;
  }
}

static const struct DeviceOps gpio_dev_ops = {
  .ioctl = gpio_dev_ioctl,
};

static struct Device gpio_dev = {
  .name = "gpio",
  .type = DEVICE_TYPE_MISC,
  .ops = &gpio_dev_ops,
};

ErrorCode gpio_register_device(void) {
  return device_register(&gpio_dev);
}
