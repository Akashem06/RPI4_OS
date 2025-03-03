#pragma once

/*******************************************************************************************************************************
 * @file   bcm2711_cpu.h
 *
 * @brief  BCM2711 CPU abstraction
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "common.h"
#include "error.h"

/* Intra-component Headers */
#include "arm64_io.h"
#include "bcm2711.h"

/**
 * @defgroup BCM2711_Hardware BCM2711 Hardware layer
 * @brief    Abstraction layer for the BCM2711 SoC from Broadcom
 * @{
 */

/**
 * @brief   Get the current CPU ID (0-3 on RPi4)
 * @return  CPU ID (0-3)
 */
extern u32 get_cpu_id(void);

/**
 * @brief   Yield the CPU, waiting for a wakeup event
 */
extern void cpu_yield(void);

/**
 * @brief   Wakeup other CPUs that are waiting for an event
 */
extern void wakeup_cpu(void);

/**
 * @brief   Put the CPU into sleep mode, waiting for an interrupt
 */
extern void cpu_wait_for_interrupt(void);

/**
 * @brief   Enable interrupts
 */
extern void cpu_enable_irq(void);

/**
 * @brief   Disable interrupts
 */
extern void cpu_disable_irq(void);

/** @} */
