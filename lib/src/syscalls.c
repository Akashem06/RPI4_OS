#include "syscalls.h"

#include <stddef.h>

#include "entry.h"
#include "log.h"
#include "mem.h"
#include "mm.h"
#include "scheduler.h"

void sys_call_write(char *buf) {
  log(buf);
}

int sys_call_clone_task(unsigned long stack) {
  return scheduler_create_task(0, 0, 0, stack);
}

unsigned long sys_call_malloc() {
  u64 addr = (u64)get_free_page();
  if (!addr) {
    return -1;
  }
  return addr;
}

void sys_call_exit() {
  scheduler_exit_task();
}

void *const sys_call_table[] = { sys_call_write, sys_call_malloc, sys_call_clone_task,
                                 sys_call_exit };
