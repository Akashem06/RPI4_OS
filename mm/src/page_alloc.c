/*******************************************************************************************************************************
 * @file   page_alloc.c
 *
 * @brief  Page allocation source file
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "assert.h"
#include "memops.h"

/* Intra-component Headers */
#include "page_alloc.h"

static void *memory_pool = NULL;  /* Memory pool */
static u64 memory_pool_size = 0U; /* Size of the memory pool */

/* Linux-style separate mem_map array for page structures */
static struct Page *mem_map = NULL;
static u32 num_pages = 0U;
static void *mem_map_area = NULL;

/* Header pool for slab/kmalloc metadata, carved out of the reserved region */
static void *header_pool = NULL;
static u64 header_pool_size = 0U;
static u32 pages_reserved = 0U; /* mem_map + header_pool, never handed to the buddy allocator */

bool initialized = false;

void *pfn_to_virt(u32 pfn) {
  return (void *)((u64)memory_pool + (pfn * PAGE_SIZE));
}

u32 virt_to_pfn(void *addr) {
  return ((u64)addr - (u64)memory_pool) / PAGE_SIZE;
}

struct Page *virt_to_page(void *addr) {
  u32 pfn = virt_to_pfn(addr);
  if (pfn >= num_pages) {
    return NULL;
  }
  return &mem_map[pfn];
}

void *page_to_virt(struct Page *page) {
  if (!page || page < mem_map || page >= (mem_map + num_pages)) {
    return NULL;
  }
  u32 pfn = page - mem_map;
  return pfn_to_virt(pfn);
}

void *get_memory_pool(void) {
  return memory_pool;
}

u64 get_memory_pool_size(void) {
  return memory_pool_size;
}

struct Page *get_mem_map(void) {
  return mem_map;
}

u32 get_num_pages(void) {
  return num_pages;
}

void *get_header_pool(void) {
  return header_pool;
}

u64 get_header_pool_size(void) {
  return header_pool_size;
}

u32 get_pages_reserved(void) {
  return pages_reserved;
}

bool is_mm_initialized(void) {
  return initialized;
}

ErrorCode mm_init(void *pool, u64 size) {
  if (initialized) return SUCCESS;

  if (pool == NULL) {
    pool = (void *)&__heap_start;
    size = (u64)&__heap_end - (u64)&__heap_start;
  }

  /* TODO: Add alignment checks */

  memory_pool = pool;
  memory_pool_size = size;

  memzero((u64)memory_pool, memory_pool_size); 

  num_pages = memory_pool_size / PAGE_SIZE;
  u64 mem_map_size = num_pages * sizeof(struct Page);

  /* Header pool follows mem_map, sized at 5% of the pool or 64 KB minimum */
  header_pool_size = memory_pool_size / 20U;
  if (header_pool_size < 64U * 1024U) {
    header_pool_size = 64U * 1024U;
  }

  u64 metadata_size = mem_map_size + header_pool_size;

  /* Metadata (mem_map + header_pool) must not dominate the pool */
  if (metadata_size >= memory_pool_size / 2U) {
    return ERR_MEM_OUT_OF_MEMORY;
  }

  mem_map = mem_map_area = memory_pool;
  memzero((u64)mem_map, mem_map_size);

  header_pool = (void *)((u64)mem_map + mem_map_size);

  for (u32 i = 0; i < num_pages; i++) {
    mem_map[i] = (struct Page){
      .is_free = false,
      .next = NULL,
      ._count = 0,
      .flags = 0,
      .freelist = NULL,
      .slab = NULL
    };
  }

  /* Reserve mem_map + header_pool so the buddy allocator never hands them out */
  pages_reserved = (metadata_size + PAGE_SIZE - 1U) / PAGE_SIZE;

  /* The header pool must end inside the reserved region, never inside a free page */
  ASSERT((u64)header_pool + header_pool_size <= (u64)memory_pool + (u64)pages_reserved * PAGE_SIZE);

  for (u32 i = pages_reserved; i < num_pages; i++) {
    mem_map[i].is_free = true;
  }

  initialized = true;
  return SUCCESS;
}
