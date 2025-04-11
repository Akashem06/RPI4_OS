/*******************************************************************************************************************************
 * @file   kernel_malloc.c
 *
 * @brief  Kernel memory allocation with Linux-style separate metadata
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "mem_utils.h"
#include "spinlock.h"

/* Intra-component Headers */
#include "kernel_malloc.h"
#include "slab.h"

/**
 * @brief   Direct allocation header
 * @details Stored outside the actual allocation
 */
struct DirectHeader {
  u32 magic; /**< Magic number for validation */
  u32 size;  /**< Size of the allocation */
  u32 order; /**< Order of the buddy allocation */
};

/* Structure to manage direct allocation mappings */
struct DirectAllocMap {
  void *addr;                  /**< User address of the allocation */
  struct DirectHeader *hdr;    /**< Header for this allocation */
  struct Page *page;           /**< Page structure for this allocation */
  struct DirectAllocMap *next; /**< Next entry in the hash */
};

#define DIRECT_ALLOC_HASH_SIZE 64
static struct DirectAllocMap *direct_alloc_hash[DIRECT_ALLOC_HASH_SIZE];

bool kmalloc_initialized = false;
static struct Spinlock kmalloc_lock = SPIN_LOCK_INIT;

/**
 * @brief   Get hash bucket for a pointer
 * @param   ptr Pointer to hash
 * @return  Hash bucket index
 */
static u32 get_hash_bucket(void *ptr) {
  return ((u64)ptr / sizeof(void *)) & (DIRECT_ALLOC_HASH_SIZE - 1);
}

/**
 * @brief   Add direct allocation to hash table
 * @param   addr User address
 * @param   hdr Header structure
 * @param   page First page of allocation
 */
static void add_direct_alloc(void *addr, struct DirectHeader *hdr, struct Page *page) {
  u32 bucket = get_hash_bucket(addr);

  struct DirectAllocMap *map = alloc_header(sizeof(struct DirectAllocMap));
  if (!map) {
    return;
  }

  map->addr = addr;
  map->hdr = hdr;
  map->page = page;
  map->next = direct_alloc_hash[bucket];
  direct_alloc_hash[bucket] = map;
}

/**
 * @brief   Find direct allocation in hash table
 * @param   addr User address
 * @return  Direct allocation map entry or NULL if not found
 */
static struct DirectAllocMap *find_direct_alloc(void *addr) {
  u32 bucket = get_hash_bucket(addr);

  struct DirectAllocMap *map = direct_alloc_hash[bucket];
  while (map) {
    if (map->addr == addr) {
      return map;
    }
    map = map->next;
  }

  return NULL;
}

/**
 * @brief   Remove direct allocation from hash table
 * @param   addr User address
 * @return  true if removed, false if not found
 */
static bool remove_direct_alloc(void *addr) {
  u32 bucket = get_hash_bucket(addr);

  struct DirectAllocMap **pp = &direct_alloc_hash[bucket];
  while (*pp) {
    if ((*pp)->addr == addr) {
      struct DirectAllocMap *temp = *pp;
      *pp = temp->next;
      /* We don't free the map entry since we don't have a way to free from header_pool */
      return true;
    }
    pp = &(*pp)->next;
  }

  return false;
}

/* Direct allocation functions */
static void *direct_alloc(size_t size) {
  u32 total_size = size;
  u32 pages_needed = (total_size + PAGE_SIZE - 1) / PAGE_SIZE;

  /* Find suitable order */
  u32 order = 0;
  while ((1U << order) < pages_needed && order < MAX_ORDER) {
    order++;
  }

  /* Allocate pages */
  struct Page *page = buddy_alloc_pages(order);
  if (!page) {
    return NULL;
  }

  /* Allocate a header in the header pool */
  struct DirectHeader *header = alloc_header(sizeof(struct DirectHeader));
  if (!header) {
    buddy_free_pages(page);
    return NULL;
  }

  header->magic = KMALLOC_MAGIC;
  header->size = size;
  header->order = order;

  /* Get the memory address for the user */
  void *addr = page_to_virt(page);

  /* Add to the direct allocation hash table */
  add_direct_alloc(addr, header, page);

  return addr;
}

static void direct_free(void *ptr) {
  struct DirectAllocMap *map = find_direct_alloc(ptr);
  if (!map) {
    return;
  }

  if (map->hdr->magic != KMALLOC_MAGIC) {
    /* Corrupted header */
    return;
  }

  /* Free the pages */
  buddy_free_pages(map->page);

  /* Remove from hash table */
  remove_direct_alloc(ptr);
}

static ErrorCode kmalloc_init(void) {
  if (kmalloc_initialized) {
    return SUCCESS;
  }

  spin_lock(&kmalloc_lock);

  for (u32 i = 0; i < DIRECT_ALLOC_HASH_SIZE; i++) {
    direct_alloc_hash[i] = NULL;
  }

  kmalloc_initialized = true;

  spin_unlock(&kmalloc_lock);

  return SUCCESS;
}

void *kmalloc(size_t size) {
  if (!is_mm_initialized()) {
    if (mm_init(NULL, 0) != SUCCESS) {
      return NULL;
    }
  }

  if (!kmalloc_initialized) {
    if (kmalloc_init() != SUCCESS) {
      return NULL;
    }
  }

  spin_lock(&kmalloc_lock);

  void *result = NULL;

  if (size > MAX_SLAB_SIZE) {
    result = direct_alloc(size);
  } else {
    result = slab_alloc(size);
  }

  spin_unlock(&kmalloc_lock);

  return result;
}

void kfree(void *ptr) {
  if (ptr == NULL || !is_mm_initialized()) {
    return;
  }

  spin_lock(&kmalloc_lock);

  struct DirectAllocMap *map = find_direct_alloc(ptr);
  if (map) {
    direct_free(ptr);
  } else {
    slab_free(ptr);
  }

  spin_unlock(&kmalloc_lock);
}

void *kzalloc(size_t size) {
  void *ptr = kmalloc(size);

  spin_lock(&kmalloc_lock);

  if (ptr) {
    memzero((u64)ptr, size);
  }

  spin_unlock(&kmalloc_lock);

  return ptr;
}
