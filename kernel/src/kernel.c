/*******************************************************************************************************************************
 * @file   kernel.c
 *
 * @brief  Shared kernel boot: core service bring-up common to every entry point
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "bcm2711_board.h"
#include "irq.h"
#include "kernel_malloc.h"
#include "log.h"
#include "uart.h"
#include "utils.h"

/* Intra-component Headers */
#include "kernel.h"

/* Kept static so example entry points can define their own global settings without collision */
static UartSettings settings = {
  .uart = UART0,
  .tx = 14,
  .rx = 15,
};

void kernel_boot(void) {
  uart_init(&settings);
  log_init(LOG_MODE_UART);

  kalloc_init();
  irq_init_vectors();

  /* Bring up the board and register its hardware behind the kernel's abstract seams */
  board_init();

  log("QEMU Test: Kernel booted at EL%d\n\r", get_el());
}
