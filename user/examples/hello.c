/*******************************************************************************************************************************
 * @file   hello.c
 *
 * @brief  Minimal EL0 user program, greets the world through the device syscalls
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "common.h"
#include "syscalls.h"

/* Intra-component Headers */

/* Runs at EL0, so it may only touch the kernel through the svc syscall wrappers */

static u64 ustrlen(const char *s) {
  u64 n = 0;
  while (s[n]) {
    n++;
  }
  return n;
}

/**
 * @brief   EL0 entry point, opens uart0 and writes a greeting via syscalls
 * @details Spawned from a kernel example with scheduler_create_user_task. It has
 *          no direct hardware access, everything goes through open/write/close.
 *          Must terminate with call_sys_exit, an EL0 thread cannot simply return.
 */
void user_hello_main(void) {
  const char *msg = "hello from EL0 (user space via syscalls)\n\r";

  long fd = call_sys_open("uart0");
  if (fd >= 0) {
    call_sys_write_dev(fd, msg, ustrlen(msg));
    call_sys_close(fd);
  }

  call_sys_exit();
}
