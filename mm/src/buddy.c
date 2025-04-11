/*******************************************************************************************************************************
 * @file   buddy.c
 *
 * @brief  Buddy memory manager source file
 *
 * @date   2025-04-07
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "spinlock.h"

/* Intra-component Headers */
#include "buddy.h"

static struct Spinlock buddy_alloc_lock = SPIN_LOCK_INIT;
static int recursion_depth = 0;

static struct Page *free_lists[MAX_ORDER + 1U]; /* Free lists for each order */
static bool buddy_initialized = false;

static struct Page *get_buddy_page(struct Page *page, u32 order) {
  u32 pfn = page - get_mem_map();
  u32 buddy_pfn = pfn ^ (1U << order);

  if (buddy_pfn >= get_num_pages()) {
    return NULL;
  }

  return &get_mem_map()[buddy_pfn];
}

ErrorCode buddy_init(void) {
  if (buddy_initialized) {
    return SUCCESS;
  }

  if (!is_mm_initialized()) {
    if (mm_init(NULL, 0U) != SUCCESS) {
      return ERR_MEM_INIT_FAILED;
    }
  }

  spin_lock(&buddy_alloc_lock);

  for (u32 i = 0U; i <= MAX_ORDER; i++) {
    free_lists[i] = NULL;
  }

  struct Page *mem_map = get_mem_map();
  u32 num_pages = get_num_pages();

  u64 mem_map_size = num_pages * sizeof(struct Page);
  u32 pages_reserved = (mem_map_size + PAGE_SIZE - 1U) / PAGE_SIZE;

  /* Create free lists */
  u32 pages_left = num_pages - pages_reserved;
  u32 start_pfn = pages_reserved;

  while (pages_left > 0) {
    u32 order = MAX_ORDER;
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

  buddy_initialized = true;

  spin_unlock(&buddy_alloc_lock);

  return SUCCESS;
}

struct Page *buddy_alloc_pages(u32 order) {
  if (!buddy_initialized) {
    if (buddy_init() != SUCCESS) {
      return NULL;
    }
  }

  if (order > MAX_ORDER) {
    return NULL;
  }

  spin_lock(&buddy_alloc_lock);

  /* Check if we have a block of the right size */
  if (free_lists[order] != NULL) {
    struct Page *page = free_lists[order];
    free_lists[order] = page->next;
    page->is_free = false;
    page->next = NULL;
    page->_count = 1; /* Set reference count */
    spin_unlock(&buddy_alloc_lock);
    return page;
  }

  /* Try to split larger blocks */
  for (u32 i = order + 1; i <= MAX_ORDER; i++) {
    if (free_lists[i] != NULL) {
      if (buddy_split_block(i) == SUCCESS) {
        /* Try again with the newly split blocks */
        spin_unlock(&buddy_alloc_lock);
        return buddy_alloc_pages(order);
      }
    }
  }

  spin_unlock(&buddy_alloc_lock);

  return NULL;
}

void buddy_free_pages(struct Page *page) {
  if (!page || !page->is_free) {
    if (page) {
      page->is_free = true;
    } else {
      return;
    }
  }

  u32 order = page->order;
  struct Page *buddy = get_buddy_page(page, order);

  spin_lock(&buddy_alloc_lock);

  while (buddy && buddy->is_free && buddy->order == order) {
    /* Remove buddy from free list */
    struct Page **pp = &free_lists[order];
    while (*pp && *pp != buddy) {
      pp = &(*pp)->next;
    }
    if (*pp) {
      *pp = buddy->next;
    }

    /* Determine which page is the lower address */
    page = (page < buddy) ? page : buddy;
    page->order = order + 1;

    buddy = get_buddy_page(page, page->order);
  }

  /* Add the page back to the free list if no merging happens */
  page->next = free_lists[order];
  free_lists[order] = page;

  spin_unlock(&buddy_alloc_lock);
}

ErrorCode buddy_split_block(u32 order) {
  if (order == 0 || order > MAX_ORDER) {
    return ERR_GEN_INVALID_PARAM;
  }

  if (free_lists[order] == NULL) {
    /* No free memory available for the given order */
    return ERR_MEM_OUT_OF_MEMORY;
  }

  struct Page *block = free_lists[order];
  free_lists[order] = block->next; /* Remove current block from free list */

  u32 lower_order = order - 1U;
  u32 pfn = block - get_mem_map();

  /* First half becomes a block of the lower order */
  block->order = lower_order;
  block->next = free_lists[lower_order];
  free_lists[lower_order] = block;

  /* Second half becomes another block of the lower order */
  struct Page *buddy = &get_mem_map()[pfn + (1 << lower_order)];
  buddy->order = lower_order;
  buddy->is_free = true;
  buddy->next = free_lists[lower_order];
  free_lists[lower_order] = buddy;

  return SUCCESS;
}
