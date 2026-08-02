/*******************************************************************************************************************************
 * @file   device.c
 *
 * @brief  Unified device interface and registry
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stddef.h>

/* Inter-component Headers */

/* Intra-component Headers */
#include "device.h"

/* Registry of all devices, guarded by registry_lock */
static struct Device *devices[MAX_DEVICES] = { NULL };
static struct Spinlock registry_lock = SPIN_LOCK_INIT;

/* Small local string compare, the kernel has no libc */
static bool name_equal(const char *a, const char *b) {
  if (!a || !b) {
    return false;
  }
  for (u32 i = 0; i < DEVICE_NAME_MAX; i++) {
    if (a[i] != b[i]) {
      return false;
    }
    if (a[i] == '\0') {
      return true;
    }
  }
  return true;
}

/* Registry lock held, find the slot holding dev or -1 */
static int find_slot_locked(const struct Device *dev) {
  for (int i = 0; i < MAX_DEVICES; i++) {
    if (devices[i] == dev) {
      return i;
    }
  }
  return -1;
}

/* Registry lock held, do the insert */
static ErrorCode device_register_locked(struct Device *dev) {
  int free_slot = -1;

  for (int i = 0; i < MAX_DEVICES; i++) {
    if (devices[i] == NULL) {
      if (free_slot < 0) {
        free_slot = i;
      }
      continue;
    }
    if (name_equal(devices[i]->name, dev->name)) {
      return ERR_DEVICE_BUSY;
    }
  }

  if (free_slot < 0) {
    return ERR_DEVICE_BUSY;
  }

  dev->refcount = 0;
  dev->lock = (struct Spinlock)SPIN_LOCK_INIT;
  devices[free_slot] = dev;
  return SUCCESS;
}

ErrorCode device_register(struct Device *dev) {
  if (!dev || !dev->name || !dev->ops) {
    return ERR_GEN_INVALID_PARAM;
  }

  u64 flags = spin_lock_irqsave(&registry_lock);
  ErrorCode ret = device_register_locked(dev);
  spin_unlock_irqrestore(&registry_lock, flags);
  return ret;
}

ErrorCode device_unregister(struct Device *dev) {
  if (!dev) {
    return ERR_GEN_INVALID_PARAM;
  }

  u64 flags = spin_lock_irqsave(&registry_lock);
  int slot = find_slot_locked(dev);
  ErrorCode ret = ERR_DEVICE_NO_DEVICE;
  if (slot >= 0) {
    devices[slot] = NULL;
    ret = SUCCESS;
  }
  spin_unlock_irqrestore(&registry_lock, flags);
  return ret;
}

struct Device *device_find(const char *name) {
  if (!name) {
    return NULL;
  }

  u64 flags = spin_lock_irqsave(&registry_lock);
  struct Device *found = NULL;
  for (int i = 0; i < MAX_DEVICES; i++) {
    if (devices[i] && name_equal(devices[i]->name, name)) {
      found = devices[i];
      break;
    }
  }
  spin_unlock_irqrestore(&registry_lock, flags);
  return found;
}

/* Device lock held, bump refcount and run ops->open on the first open */
static ErrorCode device_open_locked(struct Device *dev) {
  if (dev->refcount == 0 && dev->ops->open) {
    ErrorCode ret = dev->ops->open(dev);
    if (IS_ERROR(ret)) {
      return ret;
    }
  }
  dev->refcount++;
  return SUCCESS;
}

ErrorCode device_open(struct Device *dev) {
  if (!dev) {
    return ERR_GEN_INVALID_PARAM;
  }

  u64 flags = spin_lock_irqsave(&dev->lock);
  ErrorCode ret = device_open_locked(dev);
  spin_unlock_irqrestore(&dev->lock, flags);
  return ret;
}

/* Device lock held, drop refcount and run ops->close on the last close */
static ErrorCode device_close_locked(struct Device *dev) {
  if (dev->refcount == 0) {
    return ERR_SYS_INVALID_OP;
  }
  dev->refcount--;
  if (dev->refcount == 0 && dev->ops->close) {
    return dev->ops->close(dev);
  }
  return SUCCESS;
}

ErrorCode device_close(struct Device *dev) {
  if (!dev) {
    return ERR_GEN_INVALID_PARAM;
  }

  u64 flags = spin_lock_irqsave(&dev->lock);
  ErrorCode ret = device_close_locked(dev);
  spin_unlock_irqrestore(&dev->lock, flags);
  return ret;
}

/* read/write/ioctl dispatch straight to the driver, the op owns its own
 * concurrency so we never hold a lock across a blocking transfer */
long device_read(struct Device *dev, void *buf, u64 len) {
  if (!dev || !buf) {
    return ERR_GEN_INVALID_PARAM;
  }
  if (!dev->ops->read) {
    return ERR_SYS_NOT_SUPPORTED;
  }
  return dev->ops->read(dev, buf, len);
}

long device_write(struct Device *dev, const void *buf, u64 len) {
  if (!dev || !buf) {
    return ERR_GEN_INVALID_PARAM;
  }
  if (!dev->ops->write) {
    return ERR_SYS_NOT_SUPPORTED;
  }
  return dev->ops->write(dev, buf, len);
}

ErrorCode device_ioctl(struct Device *dev, u32 cmd, u64 arg) {
  if (!dev) {
    return ERR_GEN_INVALID_PARAM;
  }
  if (!dev->ops->ioctl) {
    return ERR_SYS_NOT_SUPPORTED;
  }
  return dev->ops->ioctl(dev, cmd, arg);
}
