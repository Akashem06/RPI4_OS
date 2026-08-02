#pragma once

/*******************************************************************************************************************************
 * @file   timer.h
 *
 * @brief  System timer header file for the BCM2711 SoC
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "base.h"
#include "common.h"
#include "error.h"

/* Intra-component Headers */

/**
 * @defgroup BCM2711_Timer BCM2711 System Timer API
 * @brief    System timer abstraction layer for the BCM2711 SoC from Broadcom
 * @{
 */

#define CLOCK_HZ 1000000 /**< System timer runs at 1 MHz */
#define NUM_TIMERS 4     /**< Number of compare channels */

/**
 * @brief   System timer register definitions
 */
typedef struct {
  reg32 control_status; /**< Control/status, match flags */
  reg32 counter_lo;     /**< Low 32 bits of the counter */
  reg32 counter_hi;     /**< High 32 bits of the counter */
  reg32 compare[4];     /**< Compare channels */
} TimerRegisters;

/**
 * @brief   State for a single compare channel
 */
typedef struct {
  u32 interval;         /**< Ticks between fires */
  u32 cur_val;          /**< Next compare value */
  void (*handler)(void);/**< Fired on match */
} Timer;

/**
 * @brief   ioctl commands for the "timer" device
 */
typedef enum {
  TIMER_IOCTL_SLEEP_MS, /**< arg = milliseconds to busy-wait */
} TimerIoctl;

#define TIMER_BASE (PBASE + 0x00003000)                     /**< Timer register base address */
#define TIMER_REGS ((volatile TimerRegisters *)(TIMER_BASE))/**< Timer register block */

/**
 * @brief   Arm a compare channel to fire a handler on a fixed interval
 * @param   timer_id Channel index
 * @param   interval Ticks between fires
 * @param   handler Callback run on each match
 */
void timer_init(u8 timer_id, u32 interval, void (*handler)(void));

/**
 * @brief   System timer interrupt handler, re-arms and dispatches channels
 */
void handle_timer_irq();

/**
 * @brief   Read the 64-bit system timer counter
 * @return  Current tick count
 */
u64 timer_get_ticks();

/**
 * @brief   Busy-wait for the given number of milliseconds
 * @param   ms Milliseconds to wait
 */
void timer_sleep(u32 ms);

/**
 * @brief   Register the system timer as the "timer" misc device
 * @return  SUCCESS or a negative ErrorCode
 */
ErrorCode timer_register_device(void);

/** @} */
