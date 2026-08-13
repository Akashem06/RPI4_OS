#pragma once

/*******************************************************************************************************************************
 * @file   irq_chip.h
 *
 * @brief  Board-agnostic interrupt-controller seam, the platform registers a concrete chip
 *
 * @date   2026-08-02
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "common.h"
#include "error.h"

/* Intra-component Headers */

/**
 * @defgroup IRQ Interrupt handling
 * @{
 */

/**
 * @brief   Interrupt controller operations, supplied by the board (e.g. the GIC)
 */
struct IrqChip {
  u32 (*acknowledge)(void);           /**< Claim the pending IRQ, returns the raw ack/IAR value */
  void (*end)(u32 iar);               /**< Signal end-of-interrupt for a prior acknowledge */
  void (*enable)(u32 intid, u8 prio); /**< Enable one interrupt line at a priority */
};

/**
 * @brief   Per-interrupt handler, invoked by the generic dispatcher
 * @param   intid Interrupt ID that fired
 * @param   ctx   Opaque context registered alongside the handler
 */
typedef void (*irq_handler_t)(u32 intid, void *ctx);

/**
 * @brief   Register the board's interrupt controller with the generic dispatcher
 * @param   chip Ops table describing acknowledge/end/enable, must outlive use
 */
void irq_set_chip(const struct IrqChip *chip);

/**
 * @brief   Enable one interrupt line through the registered chip
 * @param   intid Interrupt ID to enable
 * @param   prio  Controller priority (lower is higher priority on the GIC)
 */
void irq_enable_line(u32 intid, u8 prio);

/**
 * @brief   Bind a handler to an interrupt ID
 * @param   intid Interrupt ID to route
 * @param   fn    Handler to invoke when @p intid fires
 * @param   ctx   Opaque pointer passed back to the handler
 * @return  SUCCESS, or an error if the handler table is full
 */
ErrorCode irq_register_handler(u32 intid, irq_handler_t fn, void *ctx);

/** @} */
