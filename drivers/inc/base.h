#pragma once

#if RPI_VERSION == 3
#define PBASE 0x3F000000

// Low peripheral mode address
#elif RPI_VERSION == 4
#define PBASE 0xFE000000

#else
#define PBASE 0
#error "NO RPI_VERSION DEFINED"

#endif

#define CORE_CLOCK_SPEED 1500000000