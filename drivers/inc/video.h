#pragma once

/*******************************************************************************************************************************
 * @file   video.h
 *
 * @brief  Framebuffer / display header file for the BCM2711 SoC
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>

/* Inter-component Headers */
#include "common.h"
#include "dma.h"
#include "error.h"
#include "mailbox.h"
#include "mem_utils.h"

/* Intra-component Headers */

/**
 * @defgroup BCM2711_Video BCM2711 Framebuffer API
 * @brief    Framebuffer / display abstraction, drawing on top of the VideoCore mailbox
 * @{
 */

#define TEXT_COLOR 0xFFFFFFFF /**< Default text color */
#define BACK_COLOR 0xFF090909 /**< Default background color */

#define MB (1024 * 1024) /**< One megabyte */

#define BG32_MEM_LOCATION (LOW_MEMORY + (10 * MB))       /**< 32-bpp background scratch buffer */
#define BG8_MEM_LOCATION (BG32_MEM_LOCATION + (10 * MB)) /**< 8-bpp background scratch buffer */
#define VB_MEM_LOCATION (BG8_MEM_LOCATION + (4 * MB))    /**< Offscreen video buffer for DMA */

/**
 * @brief   Mailbox tag carrying a framebuffer width/height pair
 */
typedef struct {
  MailboxTag tag; /**< Tag header */
  u32 xres;       /**< Width in pixels */
  u32 yres;       /**< Height in pixels */
} MailboxFBSize;

/**
 * @brief   Mailbox tag carrying the framebuffer bit depth
 */
typedef struct {
  MailboxTag tag; /**< Tag header */
  u32 bpp;        /**< Bits per pixel */
} MailboxFBDepth;

/**
 * @brief   Mailbox tag carrying the framebuffer pitch
 */
typedef struct {
  MailboxTag tag; /**< Tag header */
  u32 pitch;      /**< Bytes per row */
} MailboxFBPitch;

/**
 * @brief   Mailbox tag carrying the allocated framebuffer
 */
typedef struct {
  MailboxTag tag;   /**< Tag header */
  u32 base;         /**< Framebuffer base address */
  u32 screen_size;  /**< Framebuffer size in bytes */
} MailboxFBBuffer;

/**
 * @brief   Full framebuffer setup request sent to VideoCore
 */
typedef struct {
  MailboxFBSize res;   /**< Physical resolution */
  MailboxFBSize vres;  /**< Virtual resolution */
  MailboxFBDepth depth;/**< Bit depth */
  MailboxFBBuffer buff;/**< Allocated buffer */
  MailboxFBPitch pitch;/**< Pitch */
} MailboxFBRequest;

/**
 * @brief   Mailbox tag setting the 8-bpp palette
 */
typedef struct {
  MailboxTag tag;   /**< Tag header */
  u32 offset;       /**< First palette entry */
  u32 num_entries;  /**< Number of entries */
  u32 entries[8];   /**< Palette entries */
} MailboxSetPallete;

/**
 * @brief   Resolution request passed to FB_IOCTL_SET_RESOLUTION
 */
typedef struct {
  u32 xres; /**< Width in pixels */
  u32 yres; /**< Height in pixels */
  u32 bpp;  /**< Bits per pixel */
} FbResolution;

/**
 * @brief   ioctl commands for the "fb" device
 */
typedef enum {
  FB_IOCTL_SET_RESOLUTION, /**< arg = pointer to FbResolution */
  FB_IOCTL_SET_DMA,        /**< arg = 0/1 to disable/enable DMA blits */
} FbIoctl;

/**
 * @brief   Allocate the offscreen buffers and open the DMA channel
 */
void video_init();

/**
 * @brief   Set the display resolution and depth via the VideoCore mailbox
 * @param   xres Width in pixels
 * @param   yres Height in pixels
 * @param   bpp  Bits per pixel
 */
void video_set_resolution(u32 xres, u32 yres, u32 bpp);

/**
 * @brief   Draw a filled rectangle
 * @param   x_start Left edge
 * @param   y_start Top edge
 * @param   width   Width in pixels
 * @param   height  Height in pixels
 * @param   color   Fill color
 */
void video_draw_rectangle(u32 x_start, u32 y_start, u32 width, u32 height, u32 color);

/**
 * @brief   Draw a filled circle
 * @param   x_center Center x
 * @param   y_center Center y
 * @param   radius   Radius in pixels
 * @param   color    Fill color
 */
void video_draw_sphere(u32 x_center, u32 y_center, u32 radius, u32 color);

/**
 * @brief   Draw a single character from the built-in font
 * @param   c     Character to draw
 * @param   pos_x Left edge
 * @param   pos_y Top edge
 */
void video_draw_char(char c, u32 pos_x, u32 pos_y);

/**
 * @brief   Draw a null-terminated string
 * @param   str   String to draw
 * @param   pos_x Left edge
 * @param   pos_y Top edge
 */
void video_draw_str(char *str, u32 pos_x, u32 pos_y);

/**
 * @brief   Choose whether draws go through DMA
 * @param   dma True to blit the offscreen buffer via DMA
 */
void video_set_dma(bool dma);

/**
 * @brief   Blit the offscreen buffer to the framebuffer via DMA
 */
void video_dma();

/**
 * @brief   Register the framebuffer as the "fb" misc device
 * @return  SUCCESS or a negative ErrorCode
 */
ErrorCode video_register_device(void);

/**
 * @brief   Height of a font glyph in pixels
 * @return  Glyph height
 */
u32 font_get_height();

/**
 * @brief   Width of a font glyph in pixels
 * @return  Glyph width
 */
u32 font_get_width();

/**
 * @brief   Query whether a font glyph pixel is set
 * @param   c     Character
 * @param   pos_x Glyph column
 * @param   pos_y Glyph row
 * @return  True if the pixel is set
 */
bool font_get_pixel(char c, u32 pos_x, u32 pos_y);

/** @} */
