#pragma once

/*******************************************************************************************************************************
 * @file   page_alloc.h
 *
 * @brief  Page allocation header file
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
 * @brief   Memory page object
 * @details Maintained in a separate array outside the memory pool
 */
struct Page {
  struct Page *next; /**< Next free page in buddy list */
  u32 order;         /**< Order of this page block (2^order pages) */
  bool is_free;      /**< Whether this page is free */
  u32 _count;        /**< Reference count */
  u32 flags;         /**< Page flags */
  void *freelist;    /**< For slab allocator use */
  struct Slab *slab; /**< Slab this page belongs to, NULL for buddy allocations */
};

/**
 * @brief   Convert page frame number to physical address
 * @param   pfn Page frame number
 * @return  Physical address
 */
void *pfn_to_virt(u32 pfn);

/**
 * @brief   Convert physical address to page frame number (PFN)
 * @param   addr Physical address
 * @return  Page frame number
 */
u32 virt_to_pfn(void *addr);

/**
 * @brief   Get Page structure for a physical address
 * @param   addr Physical address
 * @return  Pointer to corresponding Page structure
 */
struct Page *virt_to_page(void *addr);

/**
 * @brief   Get physical address for a Page structure
 * @param   page Page structure
 * @return  Physical address
 */
void *page_to_virt(struct Page *page);

/**
 * @brief   Get a pointer to the memory pool
 * @return  Pointer to the memory pool
 */
void *get_memory_pool(void);

/**
 * @brief   Get the size of the memory pool
 * @return  Size of the memory pool
 */
u64 get_memory_pool_size(void);

/**
 * @brief   Get a pointer to the memory map
 * @return  Pointer to the memory map
 */
struct Page *get_mem_map(void);

/**
 * @brief   Get the number of memory pages in the allocator
 * @return  Number of pages in the allocator
 */
u32 get_num_pages(void);

/**
 * @brief   Get the initialization status of the memory manager
 * @return  TRUE if memory manager is initialized
 *          FALSE if memory manager is not initialized
 */
bool is_mm_initialized(void);

/**
 * @brief   Initialize the memory manager
 * @details By leaving pool as NULL, the memory manager is initialized
 *          with an internal pool of memory
 * @param   pool Pointer to the memory pool buffer
 * @param   size Size of the memory pool
 * @return  SUCCESS if initialized succesfully
 *          ERR_GEN_INVALID_PARAM if the size is too small
 *          ERR_MEM_OUT_OF_MEMORY if the memory map is larger than a quarter of the pool
 */
ErrorCode mm_init(void *pool, u64 size);

/** @} */
