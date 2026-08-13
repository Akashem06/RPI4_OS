/*******************************************************************************************************************************
 * @file   page.c
 *
 * @brief  Single-page allocation facade over the buddy allocator
 *
 * @date   2026-08-02
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "buddy.h"
#include "page_alloc.h"

/* Intra-component Headers */
#include "page.h"

/* Single page allocator, backed by the buddy allocator in mm/ */

void *get_free_page(void) {
  struct Page *page = buddy_alloc_pages(0);
  if (!page) {
    return NULL;
  }

  return page_to_virt(page);
}

void free_page(u64 p) {
  if (!p) {
    return;
  }

  struct Page *page = virt_to_page((void *)p);
  if (page) {
    buddy_free_pages(page);
  }
}
