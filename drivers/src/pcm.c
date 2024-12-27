#include "pcm.h"

#include "gpio.h"

void pcm_init(PCMConfig *config) {
  gpio_set_function(18, GF_ALT0);  // PCM_CLK
  gpio_set_function(19, GF_ALT0);  // PCM_FS
  gpio_set_function(20, GF_ALT0);  // PCM_DIN
  gpio_set_function(21, GF_ALT0);  // PCM_DOUT

  // Disable PCM
  PCM_REGS->cs_a = 0;

  // Clear FIFOs
  PCM_REGS->cs_a |= PCM_CS_A_TXCLR | PCM_CS_A_RXCLR;

  // Configure MODE_A register
  uint32_t mode_a = 0;
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

  PCM_REGS->mode_a = mode_a;

  // Configure RXC_A and TXC_A registers
  uint32_t channel_width = config->bits_per_sample;
  uint32_t rxc_a = (1 << 31) | (channel_width << 16) | (channel_width << 0);
  uint32_t txc_a = (1 << 31) | (channel_width << 16) | (channel_width << 0);

  if (config->channel_mode == PCM_CHANNEL_STEREO) {
    rxc_a |= (1 << 30) | (channel_width << 20) | (channel_width << 4);
    txc_a |= (1 << 30) | (channel_width << 20) | (channel_width << 4);
  }

  PCM_REGS->rxc_a = rxc_a;
  PCM_REGS->txc_a = txc_a;

  // Configure DREQ_A register (DMA request levels)
  PCM_REGS->dreq_a = (64 << 24) | (64 << 16) | (64 << 8) | 64;

  // Enable PCM and FIFOs
  PCM_REGS->cs_a |= PCM_CS_A_ENABLE | PCM_CS_A_TXON | PCM_CS_A_RXON;
}

void pcm_send(u32 *data, u32 size) {
  for (u32 i = 0; i < size; i++) {
    while (!(PCM_REGS->cs_a & PCM_CS_A_TXD)) {
    }  // Wait for TX FIFO to be available
    PCM_REGS->fifo_a = data[i];
  }
}

void pcm_recv(u32 *buffer, u32 size) {
  for (u32 i = 0; i < size; i++) {
    while (!(PCM_REGS->cs_a & PCM_CS_A_RXD)) {
    }  // Wait for RX FIFO to have data
    buffer[i] = PCM_REGS->fifo_a;
  }
}

void pcm_enable_interrupt(u32 interrupt_mask) {
  PCM_REGS->inten_a |= interrupt_mask;
}

void pcm_disable_interrupt(u32 interrupt_mask) {
  PCM_REGS->inten_a &= ~interrupt_mask;
}

uint32_t pcm_get_status(void) {
  return PCM_REGS->cs_a;
}
