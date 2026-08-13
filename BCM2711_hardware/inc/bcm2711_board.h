#pragma once

/*******************************************************************************************************************************
 * @file   bcm2711_board.h
 *
 * @brief  BCM2711 board bring-up, wires concrete hardware into the kernel's abstract seams
 *
 * @date   2026-08-02
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup BCM2711_Hardware BCM2711 Hardware layer
 * @brief    Abstraction layer for the BCM2711 SoC from Broadcom
 * @{
 */

/**
 * @brief   Bring up the board and register its hardware with the kernel core
 * @details Initializes the GIC, registers it as the IrqChip, and registers the
 *          generic timer as the scheduler's tick source. Call once at boot before
 *          starting the scheduler.
 */
void board_init(void);

/** @} */
