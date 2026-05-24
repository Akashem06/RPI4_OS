#include "mem.h"

#include "buddy.h"
#include "page_alloc.h"

/* Single page allocator, backed by the buddy allocator in mm/ */

void *get_free_page() {
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
