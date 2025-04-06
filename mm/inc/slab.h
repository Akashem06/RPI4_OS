#pragma once

/*******************************************************************************************************************************
 * @file   slab.h
 *
 * @brief  Slab memory manager header file
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
#include "buddy.h"
#include "page_alloc.h"

/**
 * @defgroup MemoryManager OS Memory Manager
 * @brief    OS Memory Manager using Buddy Allocator and Slab Allocator
 * @{
 */

#define MIN_SLAB_SIZE 8               /* Minimum allocation size */
#define MAX_SLAB_SIZE (PAGE_SIZE / 4) /* Maximum slab allocation size */
#define SLAB_SIZES (MAX_SLAB_SIZE / MIN_SLAB_SIZE)
#define KMALLOC_MAGIC 0xDEADBEEF /* Magic number for validation */
/**
 * @brief   Slab allocator object
 * @details Stored at the beginning of each slab allocation
 */
struct SlabObject {
  u32 magic;                    /**< Magic number for validation */
  u32 size;                     /**< Actual size of the allocation */
  struct Slab *parent;          /**< Parent slab */
  struct SlabObject *next_free; /**< Next free object in slab */
};

/**
 * @brief   Slab object
 */
struct Slab {
  u64 object_size;              /**< Size of each object */
  u32 total_objects;            /**< Total number of objects in this slab */
  u32 free_objects;             /**< Number of free objects */
  struct SlabObject *free_list; /**< List of free objects */
  struct Slab *next;            /**< Next slab with the same object size */
  struct Page *first_page;      /**< First page in this slab */
  u32 pages;                    /**< Number of pages in this slab */
};

/**
 * @brief   Initialize the slab memory system
 * @return  SUCCESS if initialized succesfully
 *          ERR_MEM_INIT_FAILED if initialization fails
 */
ErrorCode slab_init(void);

/**
 * @brief   Allocate a slab object with a given size
 * @details This checks existing slab caches before allocating data. If no cache
 *          of the requested size exists or the current cache is full, a new cache is initialized
 * @param   size Size of the object in bytes
 */
void *slab_alloc(u32 size);

/**
 * @brief   Deallocate a slab object
 * @param   ptr Pointer to the memory address to free
 */
void slab_free(void *ptr);

/**
 * @brief   Allocate a slab header into the memory map for object reference
 * @details This appends to the end of the memory map, after the buddy page references
 * @param   size Size of the header to allocate
 */
void *alloc_header(u32 size);

/** @} */
