#include "mem.h"

void *memcpy(void *dest, const void *src, u32 n) {
    u8 *byte_dest = (u8 *) dest;
    u8 *byte_src = (u8 *) src;

    for (u32 i = 0; i < n; i++) {
        byte_dest[i] = byte_src[i];
    }
    return dest;
}