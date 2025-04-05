#pragma once

#include "base.h"

#ifndef __ASSEMBLER__
#include <stddef.h>
#include <stdint.h>

void *memcpy(void *dest, const void *src, size_t n);
void memzero(unsigned long src, unsigned int n);
void *memset(void *ptr, int value, size_t size);

#endif
