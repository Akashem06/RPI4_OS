#include "video.h"

#include "log.h"
#include "mailbox.h"
#include "timer.h"

static MailboxFBRequest fb_req;
static DmaChannel *dma;
static u8 *vid_buffer;
static u32 *bg32_buffer;
static u32 *bg8_buffer;

static bool use_dma = false;

#define BUS_ADDR(x) (((u64)x | 0x40000000UL) & ~0xC0000000UL)

#define FRAMEBUFFER ((u8 *)BUS_ADDR(fb_req.buff.base))
#define DMABUFFER ((u8 *)vid_buffer)
#define DRAWBUFFER (use_dma ? DMABUFFER : FRAMEBUFFER)

void video_init() {
  dma = dma_open_channel(CT_NORMAL);
  vid_buffer = (u8 *)VB_MEM_LOCATION;

  bg32_buffer = (u32 *)BG32_MEM_LOCATION;
  bg8_buffer = (u32 *)BG8_MEM_LOCATION;

  for (int i = 0; i < (10 * MB) / 4; i++) {
    bg32_buffer[i] = BACK_COLOR;
  }

  for (int i = 0; i < (4 * MB) / 4; i++) {
    bg8_buffer[i] = 0x01010101;
  }
}

void do_dma(void *dest, void *src, u32 total) {
  u32 start = 0;

  while (total > 0) {
    int num_bytes = total;

    if (num_bytes > 0xFFFFFF) {
      num_bytes = 0xFFFFFF;
    }

    dma_setup_mem_copy(dma, dest + start, src + start, num_bytes, 2);
    dma_start(dma);
    dma_wait(dma);

    start += num_bytes;
    total -= num_bytes;
  }
}

void video_dma() {
  do_dma(FRAMEBUFFER, DMABUFFER, fb_req.buff.screen_size);
}

void video_set_resolution(u32 xres, u32 yres, u32 bpp) {
  fb_req.res.tag.id = RPI_FIRMWARE_FRAMEBUFFER_SET_PHYSICAL_WIDTH_HEIGHT;
  fb_req.res.tag.buffer_size = 8;
  fb_req.res.tag.value_length = 8;
  fb_req.res.xres = xres;
  fb_req.res.yres = yres;

  fb_req.vres.tag.id = RPI_FIRMWARE_FRAMEBUFFER_SET_VIRTUAL_WIDTH_HEIGHT;
  fb_req.vres.tag.buffer_size = 8;
  fb_req.vres.tag.value_length = 8;
  fb_req.vres.xres = xres;
  fb_req.vres.yres = yres;

  fb_req.depth.tag.id = RPI_FIRMWARE_FRAMEBUFFER_SET_DEPTH;
  fb_req.depth.tag.buffer_size = 4;
  fb_req.depth.tag.value_length = 4;
  fb_req.depth.bpp = bpp;

  fb_req.buff.tag.id = RPI_FIRMWARE_FRAMEBUFFER_ALLOCATE;
  fb_req.buff.tag.buffer_size = 8;
  fb_req.buff.tag.value_length = 4;
  fb_req.buff.base = 16;
  fb_req.buff.screen_size = 0;

  fb_req.pitch.tag.id = RPI_FIRMWARE_FRAMEBUFFER_GET_PITCH;
  fb_req.pitch.tag.buffer_size = 4;
  fb_req.pitch.tag.value_length = 4;
  fb_req.pitch.pitch = 0;

  MailboxSetPallete palette;
  palette.tag.id = RPI_FIRMWARE_FRAMEBUFFER_SET_PALETTE;
  palette.tag.buffer_size = 40;
  palette.tag.value_length = 0;
  palette.offset = 0;
  palette.num_entries = 8;
  palette.entries[0] = 0;
  palette.entries[1] = 0xFFBB5500;
  palette.entries[2] = 0xFFFFFFFF;
  palette.entries[3] = 0xFFFF0000;
  palette.entries[4] = 0xFF00FF00;
  palette.entries[5] = 0xFF0000FF;
  palette.entries[6] = 0x55555555;
  palette.entries[7] = 0xCCCCCCCC;

  // Sets the actual resolution
  mailbox_process((MailboxTag *)&fb_req, sizeof(fb_req));

  if (bpp == 8) {
    mailbox_process((MailboxTag *)&palette, sizeof(palette));
  }

  for (u32 i = 0; i < 4; i++) {
    if (fb_req.depth.bpp == 32) {
      if (!use_dma) {
        u32 *buff = (u32 *)FRAMEBUFFER;
        for (u32 i = 0; i < fb_req.buff.screen_size / 4; i++) {
          buff[i] = bg32_buffer[i];
        }
      } else {
        do_dma((void *)BUS_ADDR(vid_buffer), bg32_buffer, fb_req.buff.screen_size);
      }
    } else if (fb_req.depth.bpp == 8) {
      if (!use_dma) {
        u32 *buff = (u32 *)FRAMEBUFFER;
        for (u32 i = 0; i < fb_req.buff.screen_size / 4; i++) {
          buff[i] = bg8_buffer[i];
        }
      } else {
        do_dma((void *)BUS_ADDR(vid_buffer), bg8_buffer, fb_req.buff.screen_size);
      }
    }

    if (use_dma) {
      video_dma();
    }

    timer_sleep(2000);
  }
}

void video_draw_pixel(u32 x, u32 y, u32 color) {
  u32 pixel_offset = (x * (fb_req.depth.bpp >> 3)) + (y * fb_req.pitch.pitch);

  if (fb_req.depth.bpp == 32) {
    u32 *buff = (u32 *)DRAWBUFFER;
    buff[pixel_offset / 4] = color;
  } else if (fb_req.depth.bpp == 16) {
    u16 *buff = (u16 *)DRAWBUFFER;
    buff[pixel_offset / 2] = color & 0xFFFF;
  } else {
    DRAWBUFFER[pixel_offset++] = (color & 0xFF);
  }

  if (use_dma) {
    video_dma();
  }
}

void video_draw_rectangle(u32 x_start, u32 y_start, u32 width, u32 height, u32 color) {
  u32 pixel_offset;

  if (fb_req.depth.bpp == 32) {
    u32 *buff = (u32 *)DRAWBUFFER;

    for (u32 y = y_start; y < (y_start + height); y++) {
      pixel_offset = (y * fb_req.pitch.pitch) + (x_start * (fb_req.depth.bpp >> 3));
      for (u32 x = 0; x < width; x++) {
        buff[(pixel_offset / 4) + x] = color;
      }
    }
  } else if (fb_req.depth.bpp == 16) {
    u16 *buff16 = (u16 *)DRAWBUFFER;
    for (u32 y = y_start; y < (y_start + height); y++) {
      pixel_offset = (y * fb_req.pitch.pitch) + (x_start * (fb_req.depth.bpp >> 3));
      for (u32 x = 0; x < width; x++) {
        buff16[(pixel_offset / 2) + x] = color & 0xFFFF;
      }
    }
  } else {
    for (u32 y = y_start; y < (y_start + height); y++) {
      pixel_offset = (y * fb_req.pitch.pitch) + x_start;
      for (u32 x = 0; x < width; x++) {
        DRAWBUFFER[pixel_offset + x] = color & 0xFF;
      }
    }
  }

  if (use_dma) {
    video_dma();
  }
}

void video_draw_sphere(u32 x_center, u32 y_center, u32 radius, u32 color) {
  u32 pixel_offset;

  for (u32 y = 0; y < (2 * radius); y++) {
    for (u32 x = 0; x < (2 * radius); x++) {
      u32 dx = x - radius;
      u32 dy = y - radius;

      if (dx * dx + dy * dy <= radius * radius) {
        u32 x_pos = x_center + dx;
        u32 y_pos = y_center + dy;

        pixel_offset = (x_pos * (fb_req.depth.bpp >> 3)) + (y_pos * fb_req.pitch.pitch);

        if (fb_req.depth.bpp == 32) {
          u32 *buffer32 = (u32 *)DRAWBUFFER;
          buffer32[pixel_offset / 4] = color;
        } else if (fb_req.depth.bpp == 16) {
          u16 *buffer16 = (u16 *)DRAWBUFFER;
          buffer16[pixel_offset / 2] = color & 0xFFFF;
        } else {
          DRAWBUFFER[pixel_offset] = color & 0xFF;
        }
      }
    }
  }

  if (use_dma) {
    video_dma();
  }
}

void video_draw_char(char c, u32 pos_x, u32 pos_y) {
  u32 text_color = TEXT_COLOR;
  u32 back_color = BACK_COLOR;

  if (fb_req.depth.bpp == 8) {
    text_color = 2;
    back_color = 1;
  }

  for (u32 y = 0; y < font_get_height(); y++) {
    for (u32 x = 0; x < font_get_width(); x++) {
      bool yes = font_get_pixel(c, x, y);
      video_draw_pixel(pos_x + x, pos_y + y, yes ? text_color : back_color);
    }
  }
}

void video_draw_str(char *str, u32 pos_x, u32 pos_y) {
  for (u32 i = 0; str[i] != 0; pos_x += (font_get_width() + 2), i++) {
    video_draw_char(str[i], pos_x, pos_y);
  }
}

void video_set_dma(bool dma) {
  use_dma = dma;
}
