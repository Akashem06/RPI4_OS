#pragma once

/*******************************************************************************************************************************
 * @file   device.h
 *
 * @brief  Device definitions
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "spinlock.h"

/* Intra-component Headers */

/**
 * @defgroup Device Device interface
 * @{
 */

struct Device {
  const char *name;
  void *private_data;
  struct DeviceOps *device_ops;
  struct Spinlock lock;
};

/** @} */
