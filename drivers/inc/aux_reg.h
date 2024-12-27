#pragma once

/*******************************************************************************************************************************
 * @file   aux_reg.h
 *
 * @brief  AUX register header file for the BCM2711 SoC
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "common.h"
#include "hardware.h"

/* Intra-component Headers */

typedef struct {
  reg32 irq_status;
  reg32 irq_enables;
  reg32 reserved[14];
  reg32 mu_io;
  reg32 mu_ier;
  reg32 mu_iir;
  reg32 mu_lcr;
  reg32 mu_mcr;
  reg32 mu_lsr;
  reg32 mu_msr;
  reg32 mu_scratch;
  reg32 mu_control;
  reg32 mu_status;
  reg32 mu_baudrate;
} AuxRegisters;

#define AUX_BASE (PERIPHERAL_BASE_ADDRESS + 0x00215000U)
#define AUX_REGS ((volatile AuxRegisters *)(AUX_BASE))

/** @} */
