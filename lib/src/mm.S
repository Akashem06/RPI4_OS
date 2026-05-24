.globl memzero
// memzero(void *addr in x0, u64 size in x1), zeroes size bytes starting at addr
memzero:
    cbz   x1, 2f          // size 0, nothing to do
1:  cmp   x1, #8
    b.lt  3f              // fewer than 8 bytes remain, handle the byte tail
    str   xzr, [x0], #8   // zero 8 bytes, post-increment
    sub   x1, x1, #8
    cbnz  x1, 1b
    ret
3:  strb  wzr, [x0], #1   // zero remaining 1..7 bytes
    subs  x1, x1, #1
    b.gt  3b
2:  ret
