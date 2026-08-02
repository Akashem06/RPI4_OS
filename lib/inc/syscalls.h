#pragma once

/*******************************************************************************************************************************
 * @file   syscalls.h
 *
 * @brief  System call numbers and EL0 entry stubs
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup Syscalls System calls
 * @brief    Thin EL0 -> EL1 bridge, including the device fd interface
 * @{
 */

#define SYS_WRITE_NUMBER 0       /**< write(buf), log a string */
#define SYS_MALLOC_NUMBER 1      /**< malloc(), get a free page */
#define SYS_CREATE_TASK_NUMBER 2 /**< create_task(func, arg, stack) */
#define SYS_EXIT_NUMBER 3        /**< exit() */
#define SYS_OPEN_NUMBER 4        /**< open(name) -> fd */
#define SYS_READ_NUMBER 5        /**< read(fd, buf, len) */
#define SYS_WRITE_DEV_NUMBER 6   /**< write(fd, buf, len) */
#define SYS_IOCTL_NUMBER 7       /**< ioctl(fd, cmd, arg) */
#define SYS_CLOSE_NUMBER 8       /**< close(fd) */

#define NUM_SYSCALLS 9 /**< Entries in sys_call_table */

#ifndef __ASSEMBLER__

#include "common.h"

/**
 * @brief   Log a null-terminated string
 * @param   buf String to log
 */
void call_sys_write(char *buf);

/**
 * @brief   Allocate a page of memory
 * @return  Page address, or a negative errno-style value
 */
u64 call_sys_malloc(void);

/**
 * @brief   Spawn a new task running func
 * @param   func  Task entry point
 * @param   arg   Argument passed to func
 * @param   stack Task stack, or 0 to let the kernel allocate one
 */
void call_sys_create_task(u64 func, u64 arg, u64 stack);

/**
 * @brief   Terminate the calling task
 */
void call_sys_exit(void);

/**
 * @brief   Open a registered device by name
 * @param   name Device name
 * @return  File descriptor, or a negative ErrorCode
 */
long call_sys_open(const char *name);

/**
 * @brief   Read from an open device
 * @param   fd  File descriptor from call_sys_open
 * @param   buf Destination buffer
 * @param   len Bytes to read
 * @return  Bytes read, or a negative ErrorCode
 */
long call_sys_read(long fd, void *buf, u64 len);

/**
 * @brief   Write to an open device
 * @param   fd  File descriptor from call_sys_open
 * @param   buf Source buffer
 * @param   len Bytes to write
 * @return  Bytes written, or a negative ErrorCode
 */
long call_sys_write_dev(long fd, const void *buf, u64 len);

/**
 * @brief   Issue a control command to an open device
 * @param   fd  File descriptor from call_sys_open
 * @param   cmd Command
 * @param   arg Command argument
 * @return  SUCCESS or a negative ErrorCode
 */
long call_sys_ioctl(long fd, u32 cmd, u64 arg);

/**
 * @brief   Close an open device
 * @param   fd File descriptor from call_sys_open
 * @return  SUCCESS or a negative ErrorCode
 */
long call_sys_close(long fd);

#endif

/** @} */
