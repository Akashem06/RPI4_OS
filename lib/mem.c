#include "mem.h"
#include "log.h"
#include "mm.h"

#define MAX_MANAGED_PAGES 512
static u8 mem_map[MAX_MANAGED_PAGES] = {0};

void *memcpy(void *dest, const void *src, u32 n) {
    u8 *byte_dest = (u8 *) dest;
    u8 *byte_src = (u8 *) src;

    for (u32 i = 0; i < n; i++) {
        byte_dest[i] = byte_src[i];
    }
    return dest;
}

void *get_free_page() {
    for (int i = 0; i < PAGING_PAGES; i++) {
        if (mem_map[i] == 0) {
            mem_map[i] = 1;
            u64 page_addr = LOW_MEMORY + i * PAGE_SIZE;

            if (page_addr < LOW_MEMORY || page_addr >= (MAX_MANAGED_PAGES*PAGE_SIZE + LOW_MEMORY)) {
                log("Error: Invalid page address calculated: 0x%ld\n\r", page_addr);
                mem_map[i] = 0;
                return 0;
            }
            return (void *)page_addr;
        }
    }
    return 0;
}

void free_page(u64 p) {
    if (p < LOW_MEMORY || p >= MAX_MANAGED_PAGES*PAGE_SIZE) {
        log("Error: Attempting to free invalid page: 0x%ld\n\r", p);
        return;
    }
    
    u64 index = (p - LOW_MEMORY) / PAGE_SIZE;
    if (index >= MAX_MANAGED_PAGES) {
        log("Error: Page index out of bounds: %d\n\r", index);
        return;
    }
    
    mem_map[index] = 0;
}
