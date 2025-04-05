/*******************************************************************************************************************************
 * @file   kernel_malloc.h
 *
 * @brief  Kernel memory allocation with Linux-style separate metadata
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stddef.h>

/* Inter-component Headers */
#include "bcm2711.h"
#include "common.h"
#include "error.h"
#include "log.h"
#include "mem_utils.h"

/* Intra-component Headers */

#define MAX_ORDER 11                  /* 2^11 pages = 8 MB */
#define MIN_SLAB_SIZE 8               /* Minimum allocation size */
#define MAX_SLAB_SIZE (PAGE_SIZE / 4) /* Maximum slab allocation size */
#define SLAB_SIZES (MAX_SLAB_SIZE / MIN_SLAB_SIZE)
#define KMALLOC_MAGIC 0xDEADBEEF /* Magic number for validation */
#define NUM_KERNEL_MALLOC_PAGES 512U
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

/* Memory pool and management structures */
static void *memory_pool = NULL;               /* Memory pool (should be provided by the system) */
static u64 memory_pool_size = 0U;              /* Size of the memory pool */
static struct Page *free_lists[MAX_ORDER + 1]; /* Free lists for each order */
static struct Slab *slab_caches[SLAB_SIZES];   /* Slab caches for different sizes */
static bool initialized = false;               /* Initialization flag */
static u8 internal_memory_pool[NUM_KERNEL_MALLOC_PAGES * PAGE_SIZE];

/* Linux-style separate mem_map array for page structures */
static struct Page *mem_map = NULL;
static u32 num_pages = 0U;
static void *mem_map_area = NULL;

/* Direct allocation header pool - like Linux's kmalloc_head pool */
static void *header_pool = NULL;
static u64 header_pool_size = 0U;
static u64 header_pool_used = 0U;

/**
 * @brief   Convert physical address to page frame number (PFN)
 * @param   addr Physical address
 * @return  Page frame number
 */
static u32 virt_to_pfn(void *addr) {
  return ((u64)addr - (u64)memory_pool) / PAGE_SIZE;
}

/**
 * @brief   Convert page frame number to physical address
 * @param   pfn Page frame number
 * @return  Physical address
 */
static void *pfn_to_virt(u32 pfn) {
  return (void *)((u64)memory_pool + (pfn * PAGE_SIZE));
}

/**
 * @brief   Get Page structure for a physical address
 * @param   addr Physical address
 * @return  Pointer to corresponding Page structure
 */
static struct Page *virt_to_page(void *addr) {
  u32 pfn = virt_to_pfn(addr);
  if (pfn >= num_pages) {
    return NULL;
  }
  return &mem_map[pfn];
}

/**
 * @brief   Get physical address for a Page structure
 * @param   page Page structure
 * @return  Physical address
 */
static void *page_to_virt(struct Page *page) {
  if (!page || page < mem_map || page >= (mem_map + num_pages)) {
    return NULL;
  }
  u32 pfn = page - mem_map;
  return pfn_to_virt(pfn);
}

/**
 * @brief Allocate a header for direct allocation
 * @param size Size of the header
 * @return Pointer to the header or NULL if out of memory
 */
static void *alloc_header(u32 size) {
  if (header_pool_used + size > header_pool_size) {
    return NULL;
  }

  void *header = (void *)((u64)header_pool + header_pool_used);
  header_pool_used += size;

  /* Align to 8 bytes */
  header_pool_used = (header_pool_used + 7) & ~7;

  return header;
}

/**
 * @brief Get hash bucket for a pointer
 * @param ptr Pointer to hash
 * @return Hash bucket index
 */
static u32 get_hash_bucket(void *ptr) {
  return ((u64)ptr / sizeof(void *)) & (DIRECT_ALLOC_HASH_SIZE - 1);
}

/**
 * @brief Add direct allocation to hash table
 * @param addr User address
 * @param hdr Header structure
 * @param page First page of allocation
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
 * @brief Find direct allocation in hash table
 * @param addr User address
 * @return Direct allocation map entry or NULL if not found
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
 * @brief Remove direct allocation from hash table
 * @param addr User address
 * @return true if removed, false if not found
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

  for (u32 i = 0U; i < DIRECT_ALLOC_HASH_SIZE; i++) {
    direct_alloc_hash[i] = NULL;
  }

  num_pages = memory_pool_size / PAGE_SIZE;

  /*
   * Allocate space for mem_map and header_pool from beginning of memory pool
   * This is similar to Linux's memblock allocator reserving space at boot time
   */
  u64 mem_map_size = num_pages * sizeof(struct Page);

  /* Reserve 5% of pool for headers or at least 64KB */
  header_pool_size = size / 20;
  if (header_pool_size < 64 * 1024) {
    header_pool_size = 64 * 1024;
  }

  /* Round up to page size */
  u64 metadata_size = (mem_map_size + header_pool_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

  /* Check if we have enough space */
  if (metadata_size >= memory_pool_size / 2) {
    return ERR_MEM_OUT_OF_MEMORY;
  }

  mem_map_area = memory_pool;
  mem_map = mem_map_area;
  header_pool = (void *)((u64)mem_map_area + mem_map_size);
  header_pool_used = 0;

  /* Adjust memory pool */
  // void *adjusted_pool = (void *)((u64)memory_pool + metadata_size);
  u64 adjusted_size = memory_pool_size - metadata_size;
  u32 adjusted_pages = adjusted_size / PAGE_SIZE;

  /* Initialize mem_map entries */
  memzero((u64)mem_map, mem_map_size);
  for (u32 i = 0; i < num_pages; i++) {
    mem_map[i].is_free = false;
    mem_map[i].next = NULL;
    mem_map[i]._count = 0;
    mem_map[i].flags = 0;
    mem_map[i].freelist = NULL;
    mem_map[i].slab = NULL;
  }

  /* Mark pages that are available */
  for (u32 i = metadata_size / PAGE_SIZE; i < num_pages; i++) {
    mem_map[i].is_free = true;
  }

  u32 max_order = 0;
  while ((1U << max_order) <= adjusted_pages && max_order <= MAX_ORDER) {
    max_order++;
  }
  max_order--;

  /* Create free lists */
  u32 pages_left = adjusted_pages;
  u32 start_pfn = metadata_size / PAGE_SIZE;

  while (pages_left > 0) {
    u32 order = max_order;
    while ((1U << order) > pages_left && order > 0) {
      order--;
    }

    u32 block_size = 1U << order;
    struct Page *page = &mem_map[start_pfn];
    page->order = order;
    page->next = free_lists[order];
    free_lists[order] = page;

    start_pfn += block_size;
    pages_left -= block_size;
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
  free_lists[order] = block->next; /* Remove current block from free list */

  u32 lower_order = order - 1;
  u32 pfn = block - mem_map;

  /* First half becomes a block of the lower order */
  block->order = lower_order;
  block->next = free_lists[lower_order];
  free_lists[lower_order] = block;

  /* Second half becomes another block of the lower order */
  struct Page *buddy = &mem_map[pfn + (1 << lower_order)];
  buddy->order = lower_order;
  buddy->is_free = true;
  buddy->next = free_lists[lower_order];
  free_lists[lower_order] = buddy;

  return SUCCESS;
}

static struct Page *get_buddy_page(struct Page *page, u32 order) {
  u32 pfn = page - mem_map;
  u32 buddy_pfn = pfn ^ (1 << order);

  if (buddy_pfn >= num_pages) {
    return NULL;
  }

  return &mem_map[buddy_pfn];
}

static struct Page *allocate_pages(u32 order) {
  if (order > MAX_ORDER) {
    return NULL;
  }

  /* Check if we have a block of the right size */
  if (free_lists[order] != NULL) {
    struct Page *page = free_lists[order];
    free_lists[order] = page->next;
    page->is_free = false;
    page->next = NULL;
    page->_count = 1; /* Set reference count */
    return page;
  }

  /* Try to split larger blocks */
  for (u32 i = order + 1; i <= MAX_ORDER; i++) {
    if (free_lists[i] != NULL) {
      if (split_block(i) == SUCCESS) {
        /* Try again with the newly split blocks */
        return allocate_pages(order);
      }
    }
  }

  return NULL;
}

static void free_pages(struct Page *page) {
  if (!page || !page->is_free) {
    if (page) {
      page->is_free = true;
    } else {
      return;
    }
  }

  u32 order = page->order;
  struct Page *buddy = get_buddy_page(page, order);

  /* Check if buddy exists and is free */
  if (buddy && buddy->is_free && buddy->order == order) {
    /* Remove buddy from free list */
    struct Page **pp = &free_lists[order];
    while (*pp && *pp != buddy) {
      pp = &(*pp)->next;
    }
    if (*pp) {
      *pp = buddy->next;
    }

    /* Determine which page is the lower address */
    struct Page *coalesced = (page < buddy) ? page : buddy;
    coalesced->order = order + 1;

    /* Recursively try to merge with the next buddy */
    free_pages(coalesced);
    return;
  }

  /* If no merge possible, add to free list */
  page->next = free_lists[order];
  free_lists[order] = page;
}

static struct Slab *initialize_slab_cache(u32 index, u64 object_size) {
  /* Calculate how many pages we need */
  u64 real_object_size = object_size + sizeof(struct SlabObject);
  u64 objects_per_page = PAGE_SIZE / real_object_size;

  if (objects_per_page == 0) {
    objects_per_page = 1;
  }

  /* Need space for Slab management structure */
  u64 total_size = sizeof(struct Slab);
  u32 pages_needed = 1;

  /* Determine total pages needed */
  while ((pages_needed * PAGE_SIZE) < (total_size + (objects_per_page * real_object_size))) {
    pages_needed++;
  }

  /* Find the order that fits our needs */
  u32 order = 0;
  while ((1U << order) < pages_needed && order < MAX_ORDER) {
    order++;
  }

  /* Allocate pages */
  struct Page *first_page = allocate_pages(order);
  if (!first_page) {
    return NULL;
  }

  /* Initialize the slab structure in header pool */
  struct Slab *slab = alloc_header(sizeof(struct Slab));
  if (!slab) {
    free_pages(first_page);
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
  u32 pfn = first_page - mem_map;
  for (u32 i = 0; i < (1U << order); i++) {
    mem_map[pfn + i].slab = slab;
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

  /* Add to slab cache */
  slab_caches[index] = slab;
  return slab;
}

static void *slab_alloc(u32 size) {
  if (size == 0) {
    return NULL;
  }

  /* Round up to MIN_SLAB_SIZE alignment */
  size = (size + MIN_SLAB_SIZE - 1) & ~(MIN_SLAB_SIZE - 1);

  /* Handle direct page allocation for large sizes */
  if (size > MAX_SLAB_SIZE) {
    /* Calculate pages needed */
    u32 total_size = size;
    u32 pages_needed = (total_size + PAGE_SIZE - 1) / PAGE_SIZE;

    /* Find suitable order */
    u32 order = 0;
    while ((1U << order) < pages_needed && order < MAX_ORDER) {
      order++;
    }

    /* Allocate pages */
    struct Page *page = allocate_pages(order);
    if (!page) {
      return NULL;
    }

    /* Allocate a header in the header pool */
    struct DirectHeader *header = alloc_header(sizeof(struct DirectHeader));
    if (!header) {
      free_pages(page);
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

  /* Handle slab allocation */
  u32 index = (size / MIN_SLAB_SIZE) - 1;

  /* No slab cache exists for this size yet */
  if (slab_caches[index] == NULL) {
    if (initialize_slab_cache(index, size) == NULL) {
      return NULL;
    }
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

static void slab_free(void *ptr) {
  if (ptr == NULL) {
    return;
  }

  /* First check if this is a direct allocation */
  struct DirectAllocMap *map = find_direct_alloc(ptr);
  if (map) {
    /* This is a direct allocation */
    if (map->hdr->magic != KMALLOC_MAGIC) {
      /* Corrupted header */
      return;
    }

    /* Free the pages */
    free_pages(map->page);

    /* Remove from hash table */
    remove_direct_alloc(ptr);
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

static ErrorCode ensure_memory_pool() {
  if (initialized) {
    return SUCCESS;
  }

  memzero((u64)internal_memory_pool, sizeof(internal_memory_pool));
  return initialize_allocator(internal_memory_pool, sizeof(internal_memory_pool));
}

void *kmalloc(size_t size) {
  if (ensure_memory_pool() != SUCCESS) {
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
