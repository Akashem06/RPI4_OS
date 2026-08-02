/*******************************************************************************************************************************
 * @file   pcm.c
 *
 * @brief  PCM/I2S audio driver for the BCM2711 SoC
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "bcm2711_periph_io.h"
#include "device.h"
#include "gpio.h"

/* Intra-component Headers */
#include "pcm.h"

void pcm_init(PCMConfig *config) {
  gpio_set_function(18, GF_ALT0);  // PCM_CLK
  gpio_set_function(19, GF_ALT0);  // PCM_FS
  gpio_set_function(20, GF_ALT0);  // PCM_DIN
  gpio_set_function(21, GF_ALT0);  // PCM_DOUT

  // Disable PCM
  REG_WR(PCM_REGS->cs_a, 0);

  // Clear FIFOs
  REG_WR(PCM_REGS->cs_a, REG_RD(PCM_REGS->cs_a) | PCM_CS_A_TXCLR | PCM_CS_A_RXCLR);

  // Configure MODE_A register
  u32 mode_a = 0;
  mode_a |= (config->bits_per_sample << PCM_MODE_A_FLEN_SHIFT);
  mode_a |= (config->clock_source == PCM_CLK_EXTERNAL) ? PCM_MODE_A_CLKM : 0;
  mode_a |= (config->sync_mode == PCM_SYNC_EXTERNAL) ? PCM_MODE_A_FSM : 0;

  switch (config->mode) {
    case PCM_MODE_I2S:
      mode_a |= (1 << 19) | (1 << 18);  // CLKI, FRMI
      break;
    case PCM_MODE_PCM:
      mode_a |= (0 << 19) | (0 << 18);  // Default PCM mode
      break;
    case PCM_MODE_TDM:
      mode_a |= (1 << 21) | (1 << 20);                     // TDM mode
      mode_a |= ((config->tdm_channels - 1) << 7);         // CH1 to CH8
      mode_a |= ((config->tdm_slot_width / 8 - 1) << 10);  // FLEN
      break;
  }

  REG_WR(PCM_REGS->mode_a, mode_a);

  // Configure RXC_A and TXC_A registers
  u32 channel_width = config->bits_per_sample;
  u32 rxc_a = (1 << 31) | (channel_width << 16) | (channel_width << 0);
  u32 txc_a = (1 << 31) | (channel_width << 16) | (channel_width << 0);

  if (config->channel_mode == PCM_CHANNEL_STEREO) {
    rxc_a |= (1 << 30) | (channel_width << 20) | (channel_width << 4);
    txc_a |= (1 << 30) | (channel_width << 20) | (channel_width << 4);
  }

  REG_WR(PCM_REGS->rxc_a, rxc_a);
  REG_WR(PCM_REGS->txc_a, txc_a);

  // Configure DREQ_A register (DMA request levels)
  REG_WR(PCM_REGS->dreq_a, (64 << 24) | (64 << 16) | (64 << 8) | 64);

  // Enable PCM and FIFOs
  REG_WR(PCM_REGS->cs_a, REG_RD(PCM_REGS->cs_a) | PCM_CS_A_ENABLE | PCM_CS_A_TXON | PCM_CS_A_RXON);
}

void pcm_send(u32 *data, u32 size) {
  for (u32 i = 0; i < size; i++) {
    while (!(REG_RD(PCM_REGS->cs_a) & PCM_CS_A_TXD)) {
    }  // Wait for TX FIFO to be available
    REG_WR(PCM_REGS->fifo_a, data[i]);
  }
}

void pcm_recv(u32 *buffer, u32 size) {
  for (u32 i = 0; i < size; i++) {
    while (!(REG_RD(PCM_REGS->cs_a) & PCM_CS_A_RXD)) {
    }  // Wait for RX FIFO to have data
    buffer[i] = REG_RD(PCM_REGS->fifo_a);
  }
}

void pcm_enable_interrupt(u32 interrupt_mask) {
  REG_WR(PCM_REGS->inten_a, REG_RD(PCM_REGS->inten_a) | interrupt_mask);
}

void pcm_disable_interrupt(u32 interrupt_mask) {
  REG_WR(PCM_REGS->inten_a, REG_RD(PCM_REGS->inten_a) & ~interrupt_mask);
}

u32 pcm_get_status(void) {
  return REG_RD(PCM_REGS->cs_a);
}

/* Device wrapper, read/write stream 32-bit audio words */

static long pcm_dev_read(struct Device *dev, void *buf, u64 len) {
  (void)dev;
  u32 words = len / sizeof(u32);
  pcm_recv((u32 *)buf, words);
  return (long)(words * sizeof(u32));
}

static long pcm_dev_write(struct Device *dev, const void *buf, u64 len) {
  (void)dev;
  u32 words = len / sizeof(u32);
  pcm_send((u32 *)buf, words);
  return (long)(words * sizeof(u32));
}

static ErrorCode pcm_dev_ioctl(struct Device *dev, u32 cmd, u64 arg) {
  (void)dev;
  switch (cmd) {
    case PCM_IOCTL_ENABLE_IRQ:
      pcm_enable_interrupt((u32)arg);
      return SUCCESS;
    case PCM_IOCTL_DISABLE_IRQ:
      pcm_disable_interrupt((u32)arg);
      return SUCCESS;
    case PCM_IOCTL_GET_STATUS:
      if (!arg) {
        return ERR_GEN_INVALID_PARAM;
      }
      *(u32 *)arg = pcm_get_status();
      return SUCCESS;
    default:
      return ERR_SYS_NOT_SUPPORTED;
  }
}

static const struct DeviceOps pcm_dev_ops = {
  .read = pcm_dev_read,
  .write = pcm_dev_write,
  .ioctl = pcm_dev_ioctl,
};

static struct Device pcm_dev = {
  .name = "pcm",
  .type = DEVICE_TYPE_CHAR,
  .ops = &pcm_dev_ops,
};

ErrorCode pcm_register_device(void) {
  return device_register(&pcm_dev);
}
