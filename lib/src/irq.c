/*******************************************************************************************************************************
 * @file   irq.c
 *
 * @brief  Board-agnostic interrupt dispatch, routes acknowledged IRQs to registered handlers
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>

/* Inter-component Headers */
#include "log.h"

/* Intra-component Headers */
#include "irq.h"

/* GICC_IAR returns an intid >= 1020 when there is nothing pending to claim */
#define IRQ_SPURIOUS_MIN 1020U
#define IRQ_INTID_MASK 0x3FFU
#define MAX_IRQ_HANDLERS 16

static const struct IrqChip *chip = NULL;

struct IrqHandlerSlot {
  u32 intid;
  irq_handler_t fn;
  void *ctx;
  bool used;
};

static struct IrqHandlerSlot handlers[MAX_IRQ_HANDLERS];

const char entry_error_messages[17][32] = {
  "SYNC_INVALID_EL1t",   "IRQ_INVALID_EL1t",   "FIQ_INVALID_EL1t",   "ERROR_INVALID_EL1T",

  "SYNC_INVALID_EL1h",   "IRQ_INVALID_EL1h",   "FIQ_INVALID_EL1h",   "ERROR_INVALID_EL1h",

  "SYNC_INVALID_EL0_64", "IRQ_INVALID_EL0_64", "FIQ_INVALID_EL0_64", "ERROR_INVALID_EL0_64",

  "SYNC_INVALID_EL0_32", "IRQ_INVALID_EL0_32", "FIQ_INVALID_EL0_32", "ERROR_INVALID_EL0_32",

  "SYSCALL_ERROR",
};

void show_invalid_entry_message(u32 type, u64 esr, u64 address, u64 fault_addr_reg, u64 stack_pointer) {
  log("ERROR CAUGHT: %s - %d. ESR: %d Address: %d\r\n", entry_error_messages[type], type, esr, address);
  log("Fault addr_reg: %d, stack pointer: %d\r\n", fault_addr_reg, stack_pointer);
}

void print_register(u64 reg_val, u64 reg_num) {
  log("REG_NUMBER: %d, VALUE: %d\n\r", reg_num, reg_val);
}

void irq_set_chip(const struct IrqChip *new_chip) {
  chip = new_chip;
}

void irq_enable_line(u32 intid, u8 prio) {
  if (chip && chip->enable) {
    chip->enable(intid, prio);
  }
}

ErrorCode irq_register_handler(u32 intid, irq_handler_t fn, void *ctx) {
  for (int i = 0; i < MAX_IRQ_HANDLERS; i++) {
    if (!handlers[i].used) {
      handlers[i].intid = intid;
      handlers[i].fn = fn;
      handlers[i].ctx = ctx;
      handlers[i].used = true;
      return SUCCESS;
    }
  }
  return ERR_GEN_NO_MEMORY;
}

static irq_handler_t lookup_handler(u32 intid, void **ctx) {
  for (int i = 0; i < MAX_IRQ_HANDLERS; i++) {
    if (handlers[i].used && handlers[i].intid == intid) {
      *ctx = handlers[i].ctx;
      return handlers[i].fn;
    }
  }
  return NULL;
}

void handle_irq(void) {
  if (!chip) {
    return;
  }

  u32 iar = chip->acknowledge();
  u32 intid = iar & IRQ_INTID_MASK;

  // Spurious interrupt, nothing was pending
  if (intid >= IRQ_SPURIOUS_MIN) {
    return;
  }

  // EOI before dispatch, a handler may context-switch and never return here
  chip->end(iar);

  void *ctx = NULL;
  irq_handler_t fn = lookup_handler(intid, &ctx);
  if (fn) {
    fn(intid, ctx);
  }
}
