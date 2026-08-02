#include <stdlib.h>

#include "common.h"
#include "irq.h"
#include "kernel.h"
#include "log.h"
#include "scheduler.h"
#include "syscalls.h"
#include "timer.h"
#include "utils.h"

UartSettings settings = {
  .uart = UART0,
  .tx = 14,
  .rx = 15,
};

void handle_uart0_irq() {}

void process(void *arg) {
  while (true) {
    log("%s", arg);
    timer_sleep(100);
  }
}

void kernel_init() {
  uart_init(&settings);
  int el = get_el();
  log("Hello! Welcome to OS schedule sample. EL: %d\n\r", el);

  irq_init_vectors();
  enable_interrupt_controller();
  irq_enable();

  scheduler_init();

  if (!current) {
    log("ERROR: current task pointer is null after scheduler init!\n\r");
    return;
  }

  long res = scheduler_create_task(PF_KTHREAD, (u64)&process, (u64) "Task 1 Priority 10\n\r", 10);
  if (res != 0) {
    log("ERROR: Failed when starting process 1. Error: %ld\n\r", res);
    return;
  }
  res = scheduler_create_task(PF_KTHREAD, (u64)&process, (u64) "Task 2 Priority 5\n\r", 5);
  if (res != 0) {
    log("ERROR: Failed when starting process 2. Error: %ld\n\r", res);
    return;
  }
  res = scheduler_create_task(PF_KTHREAD, (u64)&process, (u64) "Task 3 Priority 1\n\r", 1);
  if (res != 0) {
    log("ERROR: Failed when starting process 3. Error: %ld\n\r", res);
    return;
  }
}

void kernel_main() {
  kernel_init();

  while (1) {
  }
}
