#pragma once

#include <stdbool.h>

#include "common.h"
#include "dma.h"
#include "mailbox.h"
#include "mem_utils.h"

#define TEXT_COLOR 0xFFFFFFFF
#define BACK_COLOR 0xFF090909

#define MB (1024 * 1024)

#define BG32_MEM_LOCATION (LOW_MEMORY + (10 * MB))
#define BG8_MEM_LOCATION (BG32_MEM_LOCATION + (10 * MB))
#define VB_MEM_LOCATION (BG8_MEM_LOCATION + (4 * MB))

typedef struct {
  MailboxTag tag;
  u32 xres;
  u32 yres;
} MailboxFBSize;

typedef struct {
  MailboxTag tag;
  u32 bpp;
} MailboxFBDepth;

typedef struct {
  MailboxTag tag;
  u32 pitch;
} MailboxFBPitch;

typedef struct {
  MailboxTag tag;
  u32 base;
  u32 screen_size;
} MailboxFBBuffer;

typedef struct {
  MailboxFBSize res;
  MailboxFBSize vres;
  MailboxFBDepth depth;
  MailboxFBBuffer buff;
  MailboxFBPitch pitch;
} MailboxFBRequest;

typedef struct {
  MailboxTag tag;
  u32 offset;
  u32 num_entries;
  u32 entries[8];
} MailboxSetPallete;

void video_init();
void video_set_resolution(u32 xres, u32 yres, u32 bpp);
void video_draw_rectangle(u32 x_start, u32 y_start, u32 width, u32 height, u32 color);
void video_draw_sphere(u32 x_center, u32 y_center, u32 radius, u32 color);
void video_draw_char(char c, u32 pos_x, u32 pos_y);
void video_draw_str(char *str, u32 pos_x, u32 pos_y);
void video_set_dma(bool dma);
void video_dma();

// Comes from font_data.c
u32 font_get_height();
u32 font_get_width();
bool font_get_pixel(char c, u32 pos_x, u32 pos_y);
