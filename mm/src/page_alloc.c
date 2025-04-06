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
#include "mem_utils.h"

/* Intra-component Headers */
#include "page_alloc.h"

static void *memory_pool = NULL;  /* Memory pool (should be provided by the system) */
static u64 memory_pool_size = 0U; /* Size of the memory pool */

/* Linux-style separate mem_map array for page structures */
static struct Page *mem_map = NULL;
static u32 num_pages = 0U;
static void *mem_map_area = NULL;

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

bool is_mm_initialized(void) {
  return initialized;
}

ErrorCode mm_init(void *pool, u64 size) {
  if (initialized) {
    return SUCCESS;
  }

  if (pool == NULL) {
    pool = (void *)&__heap_start;
    size = (u64)&__heap_end - (u64)&__heap_start;
  }

  if (size < PAGE_SIZE) {
    return ERR_GEN_INVALID_PARAM;
  }

  memory_pool = pool;
  memory_pool_size = size;

  memzero((u64)memory_pool, memory_pool_size);

  num_pages = memory_pool_size / PAGE_SIZE;

  /* Reserve space for mem_map at beginning of memory pool */
  u64 mem_map_size = num_pages * sizeof(struct Page);

  /* Check if we have enough space */
  if (mem_map_size >= memory_pool_size / 4U) {
    return ERR_MEM_OUT_OF_MEMORY;
  }

  mem_map_area = memory_pool;
  mem_map = mem_map_area;

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

  /* Mark pages beyond the mem_map as available */
  u32 pages_reserved = (mem_map_size + PAGE_SIZE - 1) / PAGE_SIZE;
  for (u32 i = pages_reserved; i < num_pages; i++) {
    mem_map[i].is_free = true;
  }

  initialized = true;
  return SUCCESS;
}
