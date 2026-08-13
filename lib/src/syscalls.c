/*******************************************************************************************************************************
 * @file   syscalls.c
 *
 * @brief  System call implementations, including the device fd interface
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>

/* Inter-component Headers */
#include "device.h"
#include "entry.h"
#include "log.h"
#include "page.h"
#include "memops.h"
#include "scheduler.h"

/* Intra-component Headers */
#include "syscalls.h"

/** @brief  Max device handles open at once, shared across the single address space */
#define MAX_OPEN_FDS 16

/* fd -> device, no per-process isolation since there is no MMU */
static struct Device *fd_table[MAX_OPEN_FDS] = { NULL };

/* Validate an fd and return its device, or NULL */
static struct Device *fd_lookup(long fd) {
  if (fd < 0 || fd >= MAX_OPEN_FDS) {
    return NULL;
  }
  return fd_table[fd];
}

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

long sys_call_open(const char *name) {
  struct Device *dev = device_find(name);
  if (!dev) {
    return ERR_DEVICE_NO_DEVICE;
  }

  for (long fd = 0; fd < MAX_OPEN_FDS; fd++) {
    if (fd_table[fd] == NULL) {
      ErrorCode ret = device_open(dev);
      if (IS_ERROR(ret)) {
        return ret;
      }
      fd_table[fd] = dev;
      return fd;
    }
  }
  return ERR_DEVICE_BUSY;
}

long sys_call_read(long fd, void *buf, u64 len) {
  struct Device *dev = fd_lookup(fd);
  if (!dev) {
    return ERR_GEN_INVALID_PARAM;
  }
  return device_read(dev, buf, len);
}

long sys_call_write_dev(long fd, const void *buf, u64 len) {
  struct Device *dev = fd_lookup(fd);
  if (!dev) {
    return ERR_GEN_INVALID_PARAM;
  }
  return device_write(dev, buf, len);
}

long sys_call_ioctl(long fd, u32 cmd, u64 arg) {
  struct Device *dev = fd_lookup(fd);
  if (!dev) {
    return ERR_GEN_INVALID_PARAM;
  }
  return device_ioctl(dev, cmd, arg);
}

long sys_call_close(long fd) {
  struct Device *dev = fd_lookup(fd);
  if (!dev) {
    return ERR_GEN_INVALID_PARAM;
  }
  fd_table[fd] = NULL;
  return device_close(dev);
}

void *const sys_call_table[] = {
  sys_call_write,     sys_call_malloc, sys_call_clone_task, sys_call_exit,  sys_call_open,
  sys_call_read,      sys_call_write_dev, sys_call_ioctl,   sys_call_close,
};
