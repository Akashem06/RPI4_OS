/*******************************************************************************************************************************
 * @file   device_test.c
 *
 * @brief  Kernel-space example, exercises the unified device model and the EL0 syscall seam
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "bcm2711_gic.h"
#include "device.h"
#include "gpio.h"
#include "irq.h"
#include "log.h"
#include "scheduler.h"
#include "timer.h"
#include "uart.h"

/* Intra-component Headers */
#include "kernel.h"

/* Defined by user/examples/hello.c, launched below as an EL0 program */
extern void user_hello_main(void);

#define DEMO_GPIO_PIN 21

static void simple_delay(u32 count) {
  for (volatile u32 i = 0; i < count; i++) {
    // Busy wait
  }
}

/* Register every driver that backs this demo onto the device model */
static void register_devices(void) {
  uart_register_device();
  gpio_register_device();
  timer_register_device();
}

/* Kernel-side (EL1) tour of the unified open/read/write/ioctl/close interface */
static void device_demo(void) {
  log("\n\r===== UNIFIED DEVICE MODEL DEMO =====\n\r");

  struct Device *uart = device_find("uart0");
  if (uart && device_open(uart) == SUCCESS) {
    const char *msg = "[device] wrote to uart0 via device_write\n\r";
    device_write(uart, msg, 41);
    device_close(uart);
  }

  struct Device *timer = device_find("timer");
  if (timer && device_open(timer) == SUCCESS) {
    u64 ticks = 0;
    device_read(timer, &ticks, sizeof(ticks));
    log("[device] timer ticks = 0x%lx\n\r", ticks);
    device_ioctl(timer, TIMER_IOCTL_SLEEP_MS, 100);
    device_close(timer);
  }

  struct Device *gpio = device_find("gpio");
  if (gpio && device_open(gpio) == SUCCESS) {
    device_ioctl(gpio, GPIO_IOCTL_SET_FUNCTION, DEMO_GPIO_PIN | (GF_OUTPUT << 8));
    device_ioctl(gpio, GPIO_IOCTL_SET_HIGH, DEMO_GPIO_PIN);
    log("[device] drove gpio pin %d high via ioctl\n\r", DEMO_GPIO_PIN);
    device_close(gpio);
  }

  log("===== DEVICE MODEL DEMO COMPLETE =====\n\r");
}

/* Kernel thread, reads the timer device each pass to show devices + multitasking together */
static void ticker_thread(u64 id) {
  struct Device *timer = device_find("timer");
  u32 count = 0;

  while (1) {
    u64 ticks = 0;
    if (timer) {
      device_read(timer, &ticks, sizeof(ticks));
    }
    log("[thread %d] tick %d, timer=0x%lx\n\r", (int)id, count++, ticks);
    simple_delay(20000000);
    schedule();
  }
}

void kernel_main() {
  kernel_boot();

  register_devices();
  device_demo();

  log("\n\r===== STARTING SCHEDULER =====\n\r");
  gic_init();
  scheduler_init();

  /* One EL0 user program plus two kernel threads, all multitasking cooperatively */
  scheduler_create_user_task((u64)&user_hello_main);
  scheduler_create_task(PF_KTHREAD, (u64)&ticker_thread, 1, 6);
  scheduler_create_task(PF_KTHREAD, (u64)&ticker_thread, 2, 4);

  irq_enable();

  while (1) {
    schedule();
  }
}
