/*******************************************************************************************************************************
 * @file   slab.c
 *
 * @brief  Slab memory manager source file
 *
 * @date   2025-04-07
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stddef.h>

/* Inter-component Headers */
#include "bcm2711.h"
#include "common.h"
#include "error.h"

/* Intra-component Headers */
#include "slab.h"

static struct Slab *slab_caches[SLAB_SIZES]; /* Slab caches for different sizes */

/* Direct allocation header pool - like Linux's kmalloc_head pool */
static void *header_pool = NULL;
static u64 header_pool_size = 0U;
static u64 header_pool_used = 0U;

bool slab_initialized = false;

static struct Slab *initialize_slab_cache(u32 index, u64 object_size) {
  /* Calculate how many pages we need */
  u64 real_object_size = object_size + sizeof(struct SlabObject);
  u64 objects_per_page = PAGE_SIZE / real_object_size;

  if (objects_per_page == 0) {
    objects_per_page = 1;
  }

  /* Need space for objects */
  u32 pages_needed = 1;

  /* Determine total pages needed */
  while ((pages_needed * PAGE_SIZE) < (objects_per_page * real_object_size)) {
    pages_needed++;
  }

  /* Find the order that fits our needs */
  u32 order = 0;
  while ((1U << order) < pages_needed && order < MAX_ORDER) {
    order++;
  }

  /* Allocate pages */
  struct Page *first_page = buddy_alloc_pages(order);
  if (!first_page) {
    return NULL;
  }

  /* Initialize the slab structure in header pool */
  struct Slab *slab = alloc_header(sizeof(struct Slab));
  if (!slab) {
    buddy_free_pages(first_page);
    return NULL;
  }

  slab->object_size = object_size;
  slab->total_objects = objects_per_page;
  slab->free_objects = objects_per_page;
  slab->free_list = NULL;
  slab->next = NULL;
  slab->first_page = first_page;
  slab->pages = 1 << order;

  /* Mark the pages as belonging to this slab */
  u32 pfn = first_page - get_mem_map();
  for (u32 i = 0; i < (1U << order); i++) {
    get_mem_map()[pfn + i].slab = slab;
  }

  /* Initialize objects */
  void *data_start = page_to_virt(first_page);
  for (u32 i = 0; i < objects_per_page; i++) {
    struct SlabObject *obj = (struct SlabObject *)((u64)data_start + (i * real_object_size));
    obj->magic = KMALLOC_MAGIC;
    obj->size = object_size;
    obj->parent = slab;
    obj->next_free = slab->free_list;
    slab->free_list = obj;
  }

  return slab;
}

/* Allocate a header for slab structures */
void *alloc_header(u32 size) {
  if (header_pool_used + size > header_pool_size) {
    return NULL;
  }

  void *header = (void *)((u64)header_pool + header_pool_used);
  header_pool_used += size;

  /* Align to 8 bytes */
  header_pool_used = (header_pool_used + 7) & ~7;

  return header;
}

ErrorCode slab_init(void) {
  if (slab_initialized) {
    return SUCCESS;
  }

  if (!is_mm_initialized()) {
    if (mm_init(NULL, 0) != SUCCESS) {
      return ERR_MEM_INIT_FAILED;
    }
  }

  for (u32 i = 0; i < SLAB_SIZES; i++) {
    slab_caches[i] = NULL;
  }

  /* Calculate header pool size (5% of memory pool or at least 64KB) */
  u64 pool_size = get_memory_pool_size();
  header_pool_size = pool_size / 20;
  if (header_pool_size < 64 * 1024) {
    header_pool_size = 64 * 1024;
  }

  /* Place header pool after mem_map */
  u32 num_pages = get_num_pages();
  u64 mem_map_size = num_pages * sizeof(struct Page);
  header_pool = (void *)((u64)get_mem_map() + mem_map_size);
  header_pool_used = 0;

  slab_initialized = true;
  return SUCCESS;
}

void *slab_alloc(u32 size) {
  if (!slab_initialized) {
    if (slab_init() != SUCCESS) {
      return NULL;
    }
  }

  if (size == 0) {
    return NULL;
  }

  /* Round up to MIN_SLAB_SIZE alignment */
  size = (size + MIN_SLAB_SIZE - 1) & ~(MIN_SLAB_SIZE - 1);

  /* Handle direct page allocation for large sizes */
  if (size > MAX_SLAB_SIZE) {
    /* This should be handled by kmalloc.c's direct allocation */
    return NULL;
  }

  /* Handle slab allocation */
  u32 index = (size / MIN_SLAB_SIZE) - 1;

  /* No slab cache exists for this size yet */
  if (slab_caches[index] == NULL) {
    struct Slab *new_slab = initialize_slab_cache(index, size);
    if (new_slab == NULL) {
      return NULL;
    }
    slab_caches[index] = new_slab;
  }

  /* Find a slab with free objects */
  struct Slab *slab = slab_caches[index];
  while (slab && slab->free_objects == 0) {
    slab = slab->next;
  }

  /* No slab has free objects, create a new one */
  if (slab == NULL) {
    slab = initialize_slab_cache(index, size);
    if (slab == NULL) {
      return NULL;
    }

    slab->next = slab_caches[index];
    slab_caches[index] = slab;
  }

  /* Allocate from the slab */
  struct SlabObject *obj = slab->free_list;
  slab->free_list = obj->next_free;
  slab->free_objects--;

  return (void *)((u64)obj + sizeof(struct SlabObject));
}

void slab_free(void *ptr) {
  if (ptr == NULL) {
    return;
  }

  /* Must be a slab allocation */
  struct SlabObject *obj = (struct SlabObject *)((u64)ptr - sizeof(struct SlabObject));

  if (obj->magic != KMALLOC_MAGIC) {
    /* Invalid or corrupted object */
    return;
  }

  /* Get the slab */
  struct Slab *slab = obj->parent;
  if (!slab) {
    return;
  }

  /* Return the object to the free list */
  obj->next_free = slab->free_list;
  slab->free_list = obj;
  slab->free_objects++;

  /* TODO: If slab is entirely free, could return pages to the system */
}
