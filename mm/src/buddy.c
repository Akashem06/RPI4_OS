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

  u64 flags = spin_lock_irqsave(&buddy_alloc_lock);

  for (u32 i = 0U; i <= MAX_ORDER; i++) {
    free_lists[i] = NULL;
  }

  struct Page *mem_map = get_mem_map();
  u32 num_pages = get_num_pages();

  /* page_alloc.c owns the reservation (mem_map + header_pool), use it directly */
  u32 pages_reserved = get_pages_reserved();

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

  spin_unlock_irqrestore(&buddy_alloc_lock, flags);

  return SUCCESS;
}

/* Pull the head of free_lists[order] and mark it allocated, caller holds the lock */
static struct Page *take_free_block(u32 order) {
  struct Page *page = free_lists[order];
  if (!page) {
    return NULL;
  }

  free_lists[order] = page->next;
  page->is_free = false;
  page->next = NULL;
  page->_count = 1; /* Set reference count */

  return page;
}

/* Allocate a block of the given order, caller holds the lock */
static struct Page *buddy_alloc_pages_locked(u32 order) {
  /* Block of the right size is available */
  struct Page *page = take_free_block(order);
  if (page) {
    return page;
  }

  /* Find the smallest larger order that has a free block */
  u32 src = order + 1;
  while (src <= MAX_ORDER && free_lists[src] == NULL) {
    src++;
  }
  if (src > MAX_ORDER) {
    return NULL; /* Out of memory */
  }

  /* Split down to the requested order, then take the block */
  for (u32 j = src; j > order; j--) {
    if (buddy_split_block(j) != SUCCESS) {
      return NULL;
    }
  }

  return take_free_block(order);
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

  u64 flags = spin_lock_irqsave(&buddy_alloc_lock);
  struct Page *result = buddy_alloc_pages_locked(order);
  spin_unlock_irqrestore(&buddy_alloc_lock, flags);

  return result;
}

/* Free and coalesce a block, caller holds the lock */
static void buddy_free_pages_locked(struct Page *page) {
  page->is_free = true;
  u32 order = page->order;
  struct Page *buddy = get_buddy_page(page, order);

  /* Coalesce with free buddies of equal order, climbing as high as possible */
  while (buddy && buddy->is_free && buddy->order == order) {
    /* Remove buddy from its free list */
    struct Page **pp = &free_lists[order];
    while (*pp && *pp != buddy) {
      pp = &(*pp)->next;
    }
    if (*pp) {
      *pp = buddy->next;
    }

    /* Lower-address page becomes the merged block */
    page = (page < buddy) ? page : buddy;
    order++;
    page->order = order;

    buddy = get_buddy_page(page, order);
  }

  /* Insert the (possibly merged) block at its final order */
  page->next = free_lists[order];
  free_lists[order] = page;
}

void buddy_free_pages(struct Page *page) {
  if (!page) {
    return;
  }

  u64 flags = spin_lock_irqsave(&buddy_alloc_lock);
  buddy_free_pages_locked(page);
  spin_unlock_irqrestore(&buddy_alloc_lock, flags);
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
