#pragma once

#include "base.h"
#include "common.h"

#define PCM_CS_A_ENABLE (1 << 0)
#define PCM_CS_A_RXON (1 << 1)
#define PCM_CS_A_TXON (1 << 2)
#define PCM_CS_A_TXCLR (1 << 3)
#define PCM_CS_A_RXCLR (1 << 4)
#define PCM_CS_A_TXRST (1 << 5)
#define PCM_CS_A_RXRST (1 << 6)
#define PCM_CS_A_TXD (1 << 19)
#define PCM_CS_A_RXD (1 << 20)

#define PCM_MODE_A_CLKM (1 << 23)
#define PCM_MODE_A_FSM (1 << 22)
#define PCM_MODE_A_FLEN_SHIFT 10
#define PCM_MODE_A_FSLEN_SHIFT 0

typedef enum { PCM_MODE_I2S, PCM_MODE_PCM, PCM_MODE_TDM } PCMMode;

typedef enum { PCM_8BIT = 8, PCM_16BIT = 16, PCM_24BIT = 24, PCM_32BIT = 32 } PCMBitsPerSample;

typedef enum {
  PCM_SAMPLE_RATE_8KHZ = 8000,
  PCM_SAMPLE_RATE_16KHZ = 16000,
  PCM_SAMPLE_RATE_44KHZ = 44100,
  PCM_SAMPLE_RATE_48KHZ = 48000,
  PCM_SAMPLE_RATE_96KHZ = 96000
} PCMSampleRate;

typedef enum { PCM_CLK_INTERNAL, PCM_CLK_EXTERNAL } PCMClockSource;

typedef enum { PCM_SYNC_INTERNAL, PCM_SYNC_EXTERNAL } PCMSyncMode;

typedef enum { PCM_CHANNEL_MONO, PCM_CHANNEL_STEREO } PCMChannelMode;

typedef struct {
  PCMMode mode;
  PCMSampleRate sample_rate;
  PCMBitsPerSample bits_per_sample;
  PCMClockSource clock_source;
  PCMSyncMode sync_mode;
  PCMChannelMode channel_mode;
  uint8_t tdm_channels;    // Number of TDM channels (1-8)
  uint8_t tdm_slot_width;  // TDM slot width in bits
} PCMConfig;

typedef struct {
  reg32 cs_a;
  reg32 fifo_a;
  reg32 mode_a;
  reg32 rxc_a;
  reg32 txc_a;
  reg32 dreq_a;
  reg32 inten_a;
  reg32 intstc_a;
  reg32 gray;
} PCMRegs;

#define PCM_REGS ((PCMRegs *)(PBASE + 0x00203000))

void pcm_init(PCMConfig *config);
void pcm_send(u32 *data, u32 size);
void pcm_recv(u32 *buffer, u32 size);
void pcm_enable_interrupt(u32 interrupt_mask);
void pcm_disable_interrupt(u32 interrupt_mask);
u32 pcm_get_status(void);
