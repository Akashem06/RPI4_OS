#pragma once

/*******************************************************************************************************************************
 * @file   irq.h
 *
 * @brief  Board-agnostic interrupt API, exception masking plus the chip/handler seam
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "base.h"
#include "common.h"

/* Intra-component Headers */
#include "irq_chip.h"

/**
 * @defgroup IRQ Interrupt handling
 * @{
 */

void irq_init_vectors(void);
void irq_enable(void);
void irq_disable(void);
u64 irq_save_flags(void);           // Returns current DAIF
void irq_restore_flags(u64 flags);  // Restores DAIF saved by irq_save_flags

/** @} */
