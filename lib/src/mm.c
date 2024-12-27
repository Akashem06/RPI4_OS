#include "mm.h"

void *memcpy(void *dest, const void *src, size_t n) {
  uint8_t *d = dest;
  const uint8_t *s = src;
  while (n--) {
    *d++ = *s++;
  }
  return dest;
}

void *memset(void *ptr, int value, size_t size) {
  uint8_t *p = ptr;
  while (size--) {
    *p++ = value;
  }
  return ptr;
}