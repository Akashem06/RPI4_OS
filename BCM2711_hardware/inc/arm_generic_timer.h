#pragma once

/*******************************************************************************************************************************
 * @file   arm_generic_timer.h
 *
 * @brief  ARMv8 generic timer (CNTP) used as the scheduler preemption tick
 *
 * @date   2026-05-23
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "common.h"

/* Intra-component Headers */

/**
 * @defgroup BCM2711_Hardware BCM2711 Hardware layer
 * @brief    Abstraction layer for the BCM2711 SoC from Broadcom
 * @{
 */

/**
 * @brief   Start the generic timer firing at the given frequency
 * @param   hz Number of ticks per second
 */
void generic_timer_init(u32 hz);

/**
 * @brief   Reload the timer for the next tick, also deasserts the current one
 */
void generic_timer_rearm(void);

/** @} */
