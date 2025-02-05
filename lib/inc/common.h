#pragma once

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef volatile u32 reg32;
typedef volatile u64 reg64;

#define max(x, y) (x > y ? x : y)
#define min(x, y) (x < y ? x : y)
