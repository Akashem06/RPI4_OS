#pragma once

#include "common.h"

#define GPU_CACHED_BASE		0x40000000 // Cached memory for faster access is stored here
#define GPU_UNCACHED_BASE	0xC0000000 // Uncached memory for main memory access (critical situations)
#define GPU_MEM_BASE	GPU_UNCACHED_BASE // Peripherals will use the uncached memory

// Peripherals (DMA/GPU) communicate with the BUS ADDRESS which is calculated like this
#define BUS_ADDRESS(addr)	(((addr) & ~0xC0000000) | GPU_MEM_BASE)

void *memcpy(void *dest, const void *src, u32 n);
