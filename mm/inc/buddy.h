#pragma once

/*******************************************************************************************************************************
 * @file   buddy.h
 *
 * @brief  Buddy memory manager header file
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
#include "page_alloc.h"

/**
 * @defgroup MemoryManager OS Memory Manager
 * @brief    OS Memory Manager using Buddy Allocator and Slab Allocator
 * @{
 */

/** @brief 2^11 pages = 8 MB */
#define MAX_ORDER 11

/**
 * @brief   Initialize the buddy memory system
 * @return  SUCCESS if initialized succesfully
 *          ERR_MEM_INIT_FAILED if initialization fails
 */
ErrorCode buddy_init(void);

/**
 * @brief   Allocate a new memory page with a given order
 * @param   order Power of two order 2 ^ order
 * @return  Pointer to the allocated memory page
 *          NULL if no memory can be allocated
 */
struct Page *buddy_alloc_pages(u32 order);

/**
 * @brief   Deallocate a memory page
 * @param   page Pointer to the page information stored in the memory map
 */
void buddy_free_pages(struct Page *page);

/**
 * @brief   Split the first free block of a given order into 2 buddies
 * @return  SUCCESS if split succesfully
 *          ERR_MEM_OUT_OF_MEMORY if no free block of the given order is found
 */
ErrorCode buddy_split_block(u32 order);

/** @} */
