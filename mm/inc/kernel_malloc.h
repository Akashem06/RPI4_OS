#pragma once

/*******************************************************************************************************************************
 * @file   kernel_malloc.h
 *
 * @brief  Kernel memory allocation header file
 *
 * @date   2025-04-07
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stddef.h>

/* Inter-component Headers */
#include "common.h"
#include "error.h"
#include "hardware.h"

/* Intra-component Headers */

/**
 * @defgroup MemoryManager OS Memory Manager
 * @brief    OS Memory Manager using Buddy Allocator and Slab Allocator
 * @{
 */

/**
 * @brief   Allocate kernel memory
 * @param   size Number of bytes to allocate
 * @return  Pointer to allocated memory or NULL on failure
 */
void *kmalloc(size_t size);

/**
 * @brief   Free kernel memory
 * @param   ptr Pointer to memory to free
 */
void kfree(void *ptr);

/**
 * @brief   Allocate zeroed kernel memory
 * @param   size Number of bytes to allocate
 * @return  Pointer to allocated memory or NULL on failure
 */
void *kzalloc(size_t size);

/** @} */
