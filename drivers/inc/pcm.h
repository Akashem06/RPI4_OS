#pragma once

/*******************************************************************************************************************************
 * @file   pcm.h
 *
 * @brief  PCM/I2S audio header file for the BCM2711 SoC
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "base.h"
#include "common.h"
#include "error.h"

/* Intra-component Headers */

/**
 * @defgroup BCM2711_PCM BCM2711 PCM/I2S API
 * @brief    PCM/I2S audio abstraction layer for the BCM2711 SoC from Broadcom
 * @{
 */

#define PCM_CS_A_ENABLE (1 << 0) /**< Enable the PCM block */
#define PCM_CS_A_RXON (1 << 1)   /**< Enable reception */
#define PCM_CS_A_TXON (1 << 2)   /**< Enable transmission */
#define PCM_CS_A_TXCLR (1 << 3)  /**< Clear the TX FIFO */
#define PCM_CS_A_RXCLR (1 << 4)  /**< Clear the RX FIFO */
#define PCM_CS_A_TXRST (1 << 5)  /**< Reset the TX FIFO */
#define PCM_CS_A_RXRST (1 << 6)  /**< Reset the RX FIFO */
#define PCM_CS_A_TXD (1 << 19)   /**< TX FIFO has space */
#define PCM_CS_A_RXD (1 << 20)   /**< RX FIFO has data */

#define PCM_MODE_A_CLKM (1 << 23)      /**< Clock is an input (external) */
#define PCM_MODE_A_FSM (1 << 22)       /**< Frame sync is an input (external) */
#define PCM_MODE_A_FLEN_SHIFT 10       /**< Frame length shift */
#define PCM_MODE_A_FSLEN_SHIFT 0       /**< Frame sync length shift */

/**
 * @brief   PCM framing mode
 */
typedef enum {
  PCM_MODE_I2S, /**< I2S framing */
  PCM_MODE_PCM, /**< Plain PCM framing */
  PCM_MODE_TDM, /**< Time-division multiplexed framing */
} PCMMode;

/**
 * @brief   Sample width in bits
 */
typedef enum {
  PCM_8BIT = 8,   /**< 8-bit samples */
  PCM_16BIT = 16, /**< 16-bit samples */
  PCM_24BIT = 24, /**< 24-bit samples */
  PCM_32BIT = 32, /**< 32-bit samples */
} PCMBitsPerSample;

/**
 * @brief   Supported sample rates
 */
typedef enum {
  PCM_SAMPLE_RATE_8KHZ = 8000,   /**< 8 kHz */
  PCM_SAMPLE_RATE_16KHZ = 16000, /**< 16 kHz */
  PCM_SAMPLE_RATE_44KHZ = 44100, /**< 44.1 kHz */
  PCM_SAMPLE_RATE_48KHZ = 48000, /**< 48 kHz */
  PCM_SAMPLE_RATE_96KHZ = 96000, /**< 96 kHz */
} PCMSampleRate;

/**
 * @brief   Clock source selection
 */
typedef enum {
  PCM_CLK_INTERNAL, /**< SoC drives the clock */
  PCM_CLK_EXTERNAL, /**< External clock */
} PCMClockSource;

/**
 * @brief   Frame sync source selection
 */
typedef enum {
  PCM_SYNC_INTERNAL, /**< SoC drives frame sync */
  PCM_SYNC_EXTERNAL, /**< External frame sync */
} PCMSyncMode;

/**
 * @brief   Channel layout
 */
typedef enum {
  PCM_CHANNEL_MONO,   /**< Single channel */
  PCM_CHANNEL_STEREO, /**< Two channels */
} PCMChannelMode;

/**
 * @brief   ioctl commands for the "pcm" device
 */
typedef enum {
  PCM_IOCTL_ENABLE_IRQ,  /**< arg = interrupt mask to enable */
  PCM_IOCTL_DISABLE_IRQ, /**< arg = interrupt mask to disable */
  PCM_IOCTL_GET_STATUS,  /**< arg = pointer to u32 filled with the CS_A status */
} PCMIoctl;

/**
 * @brief   PCM instance configuration
 */
typedef struct {
  PCMMode mode;                    /**< Framing mode */
  PCMSampleRate sample_rate;       /**< Sample rate */
  PCMBitsPerSample bits_per_sample;/**< Sample width */
  PCMClockSource clock_source;     /**< Clock source */
  PCMSyncMode sync_mode;           /**< Frame sync source */
  PCMChannelMode channel_mode;     /**< Channel layout */
  u8 tdm_channels;                 /**< Number of TDM channels, 1 to 8 */
  u8 tdm_slot_width;               /**< TDM slot width in bits */
} PCMConfig;

/**
 * @brief   PCM Register definitions
 */
typedef struct {
  reg32 cs_a;     /**< Control and status */
  reg32 fifo_a;   /**< FIFO data */
  reg32 mode_a;   /**< Mode */
  reg32 rxc_a;    /**< RX config */
  reg32 txc_a;    /**< TX config */
  reg32 dreq_a;   /**< DMA request level */
  reg32 inten_a;  /**< Interrupt enable */
  reg32 intstc_a; /**< Interrupt status and clear */
  reg32 gray;     /**< Gray code control */
} PCMRegs;

#define PCM_REGS ((PCMRegs *)(PBASE + 0x00203000)) /**< PCM register block */

/**
 * @brief   Bring up the PCM block from the given configuration
 * @param   config PCM instance configuration
 */
void pcm_init(PCMConfig *config);

/**
 * @brief   Send audio samples, blocking until each word is accepted
 * @param   data Samples to send
 * @param   size Number of 32-bit words
 */
void pcm_send(u32 *data, u32 size);

/**
 * @brief   Receive audio samples, blocking until each word arrives
 * @param   buffer Destination for samples
 * @param   size   Number of 32-bit words
 */
void pcm_recv(u32 *buffer, u32 size);

/**
 * @brief   Enable PCM interrupts
 * @param   interrupt_mask Bits to enable in INTEN_A
 */
void pcm_enable_interrupt(u32 interrupt_mask);

/**
 * @brief   Disable PCM interrupts
 * @param   interrupt_mask Bits to clear in INTEN_A
 */
void pcm_disable_interrupt(u32 interrupt_mask);

/**
 * @brief   Read the PCM control/status register
 * @return  Current CS_A value
 */
u32 pcm_get_status(void);

/**
 * @brief   Register the PCM block as the "pcm" char device
 * @return  SUCCESS or a negative ErrorCode
 */
ErrorCode pcm_register_device(void);

/** @} */
