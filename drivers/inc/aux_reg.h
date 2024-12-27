#pragma once

#include "base.h"
#include "common.h"

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

#define AUX_BASE (PBASE + 0x00215000)
#define AUX_REGS ((volatile AuxRegisters *)(AUX_BASE))
