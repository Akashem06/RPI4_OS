/*******************************************************************************************************************************
 * @file   bcm2711_gic.c
 *
 * @brief  GIC-400 (ARM GICv2) interrupt controller driver for the BCM2711
 *
 * @date   2026-05-23
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "bcm2711_periph_io.h"

/* Intra-component Headers */
#include "bcm2711_gic.h"

/* Distributor registers */
#define GICD_CTLR (GICD_BASE + 0x000UL)
#define GICD_IGROUPR (GICD_BASE + 0x080UL)
#define GICD_ISENABLER (GICD_BASE + 0x100UL)
#define GICD_IPRIORITYR (GICD_BASE + 0x400UL)

/* CPU interface registers */
#define GICC_CTLR (GICC_BASE + 0x000UL)
#define GICC_PMR (GICC_BASE + 0x004UL)
#define GICC_BPR (GICC_BASE + 0x008UL)
#define GICC_IAR (GICC_BASE + 0x00CUL)
#define GICC_EOIR (GICC_BASE + 0x010UL)

void gic_init(void) {
  /* Disable while configuring */
  mmio_write((void *)GICD_CTLR, 0);
  mmio_write((void *)GICC_CTLR, 0);

  /* Put the banked SGIs/PPIs (INTID 0..31) into Group 1 so NS EL1 sees them as IRQ */
  mmio_write((void *)GICD_IGROUPR, 0xFFFFFFFFU);

  /* Allow all priorities through the CPU interface */
  mmio_write((void *)GICC_PMR, 0xFFU);
  mmio_write((void *)GICC_BPR, 0U);

  /* Enable Group 0 and Group 1 at the distributor, enable the CPU interface */
  mmio_write((void *)GICD_CTLR, 0x3U);
  mmio_write((void *)GICC_CTLR, 0x1U);
}

void gic_enable_irq(u32 intid, u8 priority) {
  u32 reg = intid / 32U;
  u32 bit = intid % 32U;

  /* Priority byte lives inside a 32 bit word, read-modify-write the right lane */
  u64 prio_word = GICD_IPRIORITYR + (intid & ~3U);
  u32 shift = (intid & 3U) * 8U;
  u32 v = mmio_read((void *)prio_word);
  v &= ~(0xFFU << shift);
  v |= ((u32)priority << shift);
  mmio_write((void *)prio_word, v);

  /* Enable the interrupt */
  mmio_write((void *)(GICD_ISENABLER + reg * 4U), (1U << bit));
}

u32 gic_acknowledge(void) {
  return mmio_read((void *)GICC_IAR);
}

void gic_end(u32 iar) {
  mmio_write((void *)GICC_EOIR, iar);
}
