/*******************************************************************************************************************************
 * @file   memops.c
 *
 * @brief  Freestanding byte-manipulation helpers (memcpy/memset), memzero lives in mm.S
 *
 * @date   2026-08-02
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "memops.h"

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