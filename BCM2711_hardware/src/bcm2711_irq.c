/*******************************************************************************************************************************
 * @file   bcm2711_irq.c
 *
 * @brief  Legacy BCM2711 GPU/ARM interrupt mux bring-up
 *
 * @date   2026-08-02
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "bcm2711_periph_io.h"

/* Intra-component Headers */
#include "bcm2711_irq.h"

void enable_interrupt_controller(void) {
#if RPI_VERSION == 4
  REG_WR(IRQ_REGS->irq0_disable_0, 0xFFFFFFFF);
  REG_WR(IRQ_REGS->irq0_disable_1, 0xFFFFFFFF);
  REG_WR(IRQ_REGS->irq0_disable_2, 0xFFFFFFFF);

  // Enable basic GPU0 interrupts
  REG_WR(IRQ_REGS->irq0_enable_0, IRQ_TIMER_0 | IRQ_TIMER_1 | IRQ_TIMER_2 | IRQ_TIMER_3 | IRQ_CODEC_0 | IRQ_CODEC_1 |
                                      IRQ_CODEC_2 | IRQ_JPEG | IRQ_ISP | IRQ_USB | IRQ_3D | IRQ_DMA_0 | IRQ_AUX);

  // Enable GPU1 interrupts
  REG_WR(IRQ_REGS->irq0_enable_1, IRQ_I2C_SPI_SLV | IRQ_PWA0 | IRQ_PWA1 | IRQ_SMI | IRQ_GPIO_0 | IRQ_GPIO_1 | IRQ_GPIO_2 |
                                      IRQ_GPIO_3 | IRQ_I2C | IRQ_SPI | IRQ_PCM | IRQ_UART_0 | IRQ_UART_2 | IRQ_UART_3 |
                                      IRQ_UART_4 | IRQ_UART_5);
#elif RPI_VERSION == 3
  REG_WR(IRQ_REGS->irq0_enable_1, REG_RD(IRQ_REGS->irq0_enable_1) | AUX_IRQ);
#endif
}
