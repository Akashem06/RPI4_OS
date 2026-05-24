#include "syscalls.h"

#include <stddef.h>

#include "entry.h"
#include "log.h"
#include "mem.h"
#include "mem_utils.h"
#include "scheduler.h"

void sys_call_write(char *buf) {
  log(buf);
}

int sys_call_clone_task(unsigned long stack) {
  // User-thread clone, the child inherits the parent register frame (including
  // x10/x11 = func/arg staged by call_sys_create_task) and resumes after the svc.
  // The 4th argument is the priority, the previous code wrongly passed the stack here.
  // TODO: thread the user stack through, scheduler_create_task currently uses its own page.
  (void)stack;
  return scheduler_create_task(0, 0, 0, DEFAULT_PRIORITY);
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

void *const sys_call_table[] = { sys_call_write, sys_call_malloc, sys_call_clone_task, sys_call_exit };
