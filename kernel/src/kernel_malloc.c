/*******************************************************************************************************************************
 * @file   kernel_malloc.h
 *
 * @brief  Kernel memory allocation
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stddef.h>

/* Inter-component Headers */
#include "common.h"
#include "error.h"
#include "mem_utils.h"

/* Intra-component Headers */

/** @brief  Memory page size of 4096 Kbytes */
#define PAGE_SIZE 4096U
#define MAX_ORDER 11                  /* 2^11 pages = 8 MB */
#define MIN_SLAB_SIZE 8               /* Minimum allocation size */
#define MAX_SLAB_SIZE (PAGE_SIZE / 4) /* Maximum slab allocation size */
#define SLAB_SIZES (MAX_SLAB_SIZE / MIN_SLAB_SIZE)
#define KMALLOC_MAGIC 0xDEADBEEF /* Magic number for validation */

/**
 * @brief   Memory page object
 */
struct Page {
  struct Page *next; /**< Next free page in buddy list */
  u32 order;         /**< Order of this page block (2^order pages) */
  bool is_free;      /**< Whether this page is free */
};

/**
 * @brief   Slab allocator object
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
  void *page_addr;              /**< Address of the underlying page(s) */
  struct SlabObject *free_list; /**< List of free objects */
  struct Slab *next;            /**< Next slab with the same object size */
};

static void *memory_pool = NULL;               /* Memory pool (should be provided by the system) */
static u64 memory_pool_size = 0U;              /* Size of the memory pool */
static struct Page *free_lists[MAX_ORDER + 1]; /* Free lists for each order */
static struct Slab *slab_caches[SLAB_SIZES];   /* Slab caches for different sizes */
static bool initialized = false;               /* Initialization flag */

static ErrorCode initialize_allocator(void *pool, u64 size) {
  if (pool == NULL || size < PAGE_SIZE) {
    return ERR_GEN_INVALID_PARAM;
  }

  memory_pool = pool;
  memory_pool_size = size;

  memzero((u64)memory_pool, memory_pool_size);

  for (u32 i = 0U; i <= MAX_ORDER; i++) {
    free_lists[i] = NULL;
  }

  for (u32 i = 0U; i < SLAB_SIZES; i++) {
    slab_caches[i] = NULL;
  }

  u32 num_pages = memory_pool_size / PAGE_SIZE;

  u32 max_order = 0U;

  /* Get the maximum order by comparing 2^(order) to the number of pages */
  while ((1U << max_order) <= num_pages && max_order <= MAX_ORDER) {
    max_order++;
  }

  /* The loop goes over the true maximum order, so decrement one */
  max_order--;

  u32 pages_left = num_pages;
  void *current_addr = memory_pool;

  /* Divide all pages into power of two buddies. Creates a default free list */
  while (pages_left > 0U) {
    u32 order = max_order;

    while (order < MAX_ORDER) {
      if ((1U << order) <= pages_left) {
        break;
      }
      order--;
    }

    struct Page *page_block = (struct Page *)current_addr;
    page_block->order = order;
    page_block->is_free = true;
    page_block->next = free_lists[order];
    free_lists[order] = page_block;

    pages_left -= (1U << order);
    current_addr = (void *)((u64)current_addr + ((1U << order) * PAGE_SIZE));
  }

  initialized = true;
  return SUCCESS;
}

static ErrorCode split_block(u32 order) {
  if (order == 0 || order > MAX_ORDER) {
    return ERR_GEN_INVALID_PARAM;
  }

  if (free_lists[order] == NULL) {
    /* No free memory available for the given order */
    return ERR_MEM_OUT_OF_MEMORY;
  }

  struct Page *block = free_lists[order];
  free_lists[order] = block->next; /* Remove current free block and assign to next node */

  u32 lower_order = order - 1U;
  u64 lower_size = PAGE_SIZE * (1 << lower_order);

  block->order = lower_order;
  block->next = free_lists[lower_order];
  free_lists[lower_order] = block;

  struct Page *buddy = (struct Page *)((u64)block + lower_size);
  buddy->order = lower_order;
  buddy->is_free = true;
  buddy->next = free_lists[lower_order];
  free_lists[lower_order] = buddy;

  return SUCCESS;
}

static void *allocate_pages(u32 order) {
  if (order > MAX_ORDER) {
    return NULL;
  }

  if (free_lists[order] != NULL) {
    /* Available memory */
    struct Page *block = free_lists[order];
    free_lists[order] = block->next;
    block->is_free = false;
    block->next = NULL;
    return block;
  }

  /* Search for larger order and split it */
  for (u32 i = order + 1U; i <= MAX_ORDER; i++) {
    /* This will split a block of this order, and update the free list */
    ErrorCode error = split_block(i);

    /* Recursively call this function to allocate memory */
    if (error == SUCCESS) {
      return allocate_pages(order);
    }
  }

  return NULL;
}

static void *get_buddy(void *addr, u32 order) {
  u64 block_size = PAGE_SIZE * (1U << order);

  /* We can XOR because buddies share the same prefix in the address, but differ in the bit
   * corresponding to block size */
  /* Example: Addr = 0x12000. Order = 1, meaning block is of size 4096 * 2 = 8192 */
  /* 0x12000 ^ 0x2000 = 0x14000 */
  return (void *)((u64)addr ^ block_size);
}

static bool is_valid_page(void *ptr) {
  return ptr >= memory_pool && ptr < (void *)((u64)memory_pool + memory_pool_size);
}

static void free_pages(void *ptr, u32 order) {
  if (!is_valid_page(ptr) || order > MAX_ORDER) {
    return;
  }

  struct Page *block = (struct Page *)ptr;

  block->is_free = true;

  void *buddy_addr = get_buddy(ptr, order);

  if (is_valid_page(buddy_addr)) {
    struct Page *buddy = (struct Page *)buddy_addr;

    /* If the buddy is free and the same size we can merge */
    if (buddy->is_free && buddy->order == order) {
      /* Remove buddy from linked list with double pointer method */
      struct Page **pp = &free_lists[order];

      while (*pp && *pp != buddy) {
        pp = &(*pp)->next;
      }

      if (*pp) {
        *pp = buddy->next;
      }

      struct Page *merged_block = (block < buddy) ? block : buddy;

      merged_block->order = order + 1U;

      /* Recursively call to delete free the buddy and check if we can merge more */
      free_pages(merged_block, order + 1U);
      return;
    }
  }

  /* If we couldn't merge the removed block, add to the free list */
  block->next = free_lists[order];
  free_lists[order] = block;
}

static struct Slab *initialize_slab_cache(u32 index, u64 object_size) {
  /* Calculate the number of pages for the slab by adding the header information to the size */
  u64 real_object_size = object_size + sizeof(struct SlabObject);
  u64 objects_per_page = PAGE_SIZE / real_object_size;

  if (objects_per_page == 0U) {
    objects_per_page = 1U;
  }

  u32 order = 0U;

  u64 total_size = real_object_size * objects_per_page;

  while ((PAGE_SIZE * (1U << order)) < total_size && order < MAX_ORDER) {
    order++;
  }

  void *pages = allocate_pages(order);

  if (!pages) {
    return NULL;
  }

  struct Slab *slab = (struct Slab *)pages;
  slab->object_size = object_size;
  slab->total_objects = objects_per_page;
  slab->free_objects = objects_per_page;
  slab->page_addr = pages;
  slab->next = NULL;

  void *data_start = (void *)((u64)pages + sizeof(struct Slab));
  slab->free_list = NULL;

  for (u32 i = 0U; i < objects_per_page; i++) {
    struct SlabObject *obj = (struct SlabObject *)((u64)data_start + (i * real_object_size));
    obj->magic = KMALLOC_MAGIC;
    obj->size = object_size;
    obj->parent = slab;
    obj->next_free = slab->free_list;
    slab->free_list = obj;
  }

  /* Add to the slab cache*/
  slab_caches[index] = slab;

  return slab;
}

static void *slab_alloc(u32 size) {
  if (size == 0U) {
    return NULL;
  }

  /* Rounds up to the minimum slab size */
  size = (size + MIN_SLAB_SIZE - 1U) & ~(MIN_SLAB_SIZE - 1U);

  /* Too large for slab, we must use buddy allocation */
  if (size > MAX_SLAB_SIZE) {
    u32 total_size = size + sizeof(struct SlabObject);

    u32 order = 0U;

    while ((PAGE_SIZE * (1U << order)) < total_size && order < MAX_ORDER) {
      order++;
    }

    void *pages = allocate_pages(order);

    if (pages == NULL) {
      return NULL;
    }

    struct SlabObject *obj = (struct SlabObject *)pages;
    obj->magic = KMALLOC_MAGIC;
    obj->size = size;
    obj->parent = NULL; /* No parent since its direct allocation */
    obj->next_free = NULL;

    return (void *)((u64)obj + sizeof(struct SlabObject));
  }

  u32 index = (size / MIN_SLAB_SIZE) - 1U;

  /* Slab cache is empty for this size */
  if (slab_caches[index] == NULL) {
    if (initialize_slab_cache(index, size) == NULL) {
      return NULL;
    }
  }

  /* Find a slab in the cache with free objects */

  struct Slab *slab = slab_caches[index];
  while (slab && slab->free_objects == 0U) {
    slab = slab->next;
  }

  /* If every single slab is full, create a new one */
  if (slab == NULL) {
    slab = initialize_slab_cache(index, size);

    if (slab == NULL) {
      return NULL;
    }

    slab->next = slab_caches[index];
    slab_caches[index] = slab;
  }

  struct SlabObject *obj = slab->free_list;
  slab->free_list = obj->next_free;
  slab->free_objects--;

  return (void *)((u64)obj + sizeof(struct SlabObject));
}

static void slab_free(void *ptr) {
  // Get the metadata of the allocated object
  // Add the object back to the free list of its parent slab
  // If this was a direct page allocation, free it using the buddy allocator
  if (ptr == NULL) {
    return;
  }

  struct SlabObject *obj = (struct SlabObject *)((u64)ptr - sizeof(struct SlabObject));

  if (obj->magic != KMALLOC_MAGIC) {
    /* Invalid object */
    return;
  }

  /* Direct page allocation */
  if (obj->parent == NULL) {
    u32 total_size = obj->size + sizeof(struct SlabObject);
    u32 order = 0U;

    while ((PAGE_SIZE * (1U << order)) < total_size && order < MAX_ORDER) {
      order++;
    }

    free_pages(obj, order);
  }

  /* Return the object to the slab */
  struct Slab *slab = obj->parent;
  obj->next_free = slab->free_list;
  slab->free_list = obj;
  slab->free_objects++;

  /* TODO: If slab is completely free, we could remove the entire slab */
}

static void *ensure_memory_pool(void) {
  if (!initialized) {
    static u8 dummy_pool[16U * 1024U * 1024U];
    ErrorCode error = initialize_allocator(dummy_pool, sizeof(dummy_pool));

    if (error != SUCCESS) {
      return NULL;
    }
  }

  return memory_pool;
}

void *kmalloc(size_t size) {
  if (ensure_memory_pool() == NULL) {
    return NULL;
  }

  return slab_alloc(size);
}

void kfree(void *ptr) {
  if (ptr == NULL || !initialized) {
    return;
  }

  slab_free(ptr);
}

void *kzalloc(size_t size) {
  void *ptr = kmalloc(size);
  if (ptr) {
    memzero((u64)ptr, size);
  }
  return ptr;
}
