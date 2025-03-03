#pragma once

/*******************************************************************************************************************************
 * @file   kernel_malloc.h
 *
 * @brief  Kernel memory allocation
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>

/* Inter-component Headers */
#include "error.h"

/* Intra-component Headers */

/**
 * @defgroup KernelMalloc Kernel Memory Allocator
 * @brief    Kernel Memory Allocator using Buddy Allocator and Slab Allocator
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
