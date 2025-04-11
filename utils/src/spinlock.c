/*******************************************************************************************************************************
 * @file   spinlock.c
 *
 * @brief  Spinlock API
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "spinlock.h"

#ifdef ARCH_ARM64
#include "arm64_spinlock.h"
#elif defined(ARCH_X86)
#include "x86_spinlock.h"
#else
#error "Unsupported architecture"
#endif
