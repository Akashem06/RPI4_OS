/*******************************************************************************************************************************
 * @file   bcm2711_board.c
 *
 * @brief  BCM2711 board bring-up, wires concrete hardware into the kernel's abstract seams
 *
 * @date   2026-08-02
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "arm_generic_timer.h"
#include "bcm2711_gic.h"

/* Intra-component Headers */
#include "bcm2711_board.h"

void board_init(void) {
  /* Bring up the interrupt controller and expose it through the abstract seams */
  gic_init();
  bcm2711_irq_chip_register();
  bcm2711_tick_source_register();
}
