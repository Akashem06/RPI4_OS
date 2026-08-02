#pragma once

/*******************************************************************************************************************************
 * @file   device.h
 *
 * @brief  Unified device interface and registry
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "common.h"
#include "error.h"
#include "spinlock.h"

/* Intra-component Headers */

/**
 * @defgroup Device Device interface
 * @brief    One interface every driver implements, dispatched through a name-keyed registry
 * @{
 */

/** @brief  Max devices that can be registered at once */
#define MAX_DEVICES 16

/** @brief  Longest device name, including the null terminator */
#define DEVICE_NAME_MAX 16

/**
 * @brief   Class of device, mirrors the classic char/block split
 */
typedef enum {
  DEVICE_TYPE_CHAR,  /**< Byte stream, e.g. uart */
  DEVICE_TYPE_BLOCK, /**< Fixed-size blocks, e.g. sd card */
  DEVICE_TYPE_MISC,  /**< Everything else, e.g. gpio, timer */
} DeviceType;

struct Device;

/**
 * @brief   Operations a driver provides, any op may be NULL if unsupported
 */
struct DeviceOps {
  ErrorCode (*open)(struct Device *dev);                          /**< Called on first open */
  ErrorCode (*close)(struct Device *dev);                         /**< Called on last close */
  long (*read)(struct Device *dev, void *buf, u64 len);           /**< Returns bytes read or negative ErrorCode */
  long (*write)(struct Device *dev, const void *buf, u64 len);    /**< Returns bytes written or negative ErrorCode */
  ErrorCode (*ioctl)(struct Device *dev, u32 cmd, u64 arg);       /**< Driver-specific control */
};

/**
 * @brief   A registered device, one per driver instance
 */
struct Device {
  const char *name;             /**< Registry key, must be unique */
  DeviceType type;              /**< Device class */
  void *private_data;           /**< Driver-owned state */
  const struct DeviceOps *ops;  /**< Driver operations */
  struct Spinlock lock;         /**< Guards refcount and open/close */
  u32 refcount;                 /**< Open handles referencing this device */
};

/**
 * @brief   Register a device so it can be found by name
 * @param   dev Device to register, must have a unique name
 * @return  SUCCESS, or ERR_DEVICE_BUSY if the name is taken or the table is full
 */
ErrorCode device_register(struct Device *dev);

/**
 * @brief   Remove a device from the registry
 * @param   dev Device to remove
 * @return  SUCCESS, or ERR_DEVICE_NO_DEVICE if it was not registered
 */
ErrorCode device_unregister(struct Device *dev);

/**
 * @brief   Look up a registered device by name
 * @param   name Device name
 * @return  The device, or NULL if no device has that name
 */
struct Device *device_find(const char *name);

/**
 * @brief   Open a device, bumping its refcount and running ops->open on first open
 * @param   dev Device to open
 * @return  SUCCESS or a negative ErrorCode
 */
ErrorCode device_open(struct Device *dev);

/**
 * @brief   Close a device, dropping its refcount and running ops->close on last close
 * @param   dev Device to close
 * @return  SUCCESS or a negative ErrorCode
 */
ErrorCode device_close(struct Device *dev);

/**
 * @brief   Read from a device
 * @param   dev Device to read from
 * @param   buf Destination buffer
 * @param   len Bytes to read
 * @return  Bytes read, or a negative ErrorCode
 */
long device_read(struct Device *dev, void *buf, u64 len);

/**
 * @brief   Write to a device
 * @param   dev Device to write to
 * @param   buf Source buffer
 * @param   len Bytes to write
 * @return  Bytes written, or a negative ErrorCode
 */
long device_write(struct Device *dev, const void *buf, u64 len);

/**
 * @brief   Issue a driver-specific control command
 * @param   dev Device to control
 * @param   cmd Command
 * @param   arg Command argument
 * @return  SUCCESS or a negative ErrorCode
 */
ErrorCode device_ioctl(struct Device *dev, u32 cmd, u64 arg);

/** @} */
