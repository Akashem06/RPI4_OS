#pragma once

/*******************************************************************************************************************************
 * @file   bcm2711_gic.h
 *
 * @brief  GIC-400 (ARM GICv2) interrupt controller driver for the BCM2711
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

/* GIC-400 lives in the ARM local region, not the 0xFE peripheral window */
#define GIC_BASE 0xFF840000UL
#define GICD_BASE (GIC_BASE + 0x1000UL) /* Distributor */
#define GICC_BASE (GIC_BASE + 0x2000UL) /* CPU interface */

/* Non-secure EL1 physical timer (CNTP) interrupt, PPI INTID 30 */
#define GENERIC_TIMER_PPI 30U

/* Returned by GICC_IAR when there is no pending interrupt */
#define GIC_SPURIOUS_INTID 1023U

/**
 * @brief   Initialize the GIC-400 distributor and CPU interface
 */
void gic_init(void);

/**
 * @brief   Enable a single interrupt and give it a priority
 * @param   intid    Interrupt ID (PPI/SPI)
 * @param   priority GIC priority, lower value is higher priority
 */
void gic_enable_irq(u32 intid, u8 priority);

/**
 * @brief   Acknowledge the highest priority pending interrupt
 * @return  The GICC_IAR value, low 10 bits are the interrupt ID
 */
u32 gic_acknowledge(void);

/**
 * @brief   Signal end of interrupt
 * @param   iar The value previously returned by gic_acknowledge
 */
void gic_end(u32 iar);

/**
 * @brief   Register the GIC-400 as the board's IrqChip with the generic dispatcher
 */
void bcm2711_irq_chip_register(void);

/** @} */
