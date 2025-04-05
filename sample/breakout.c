#include <stdlib.h>

#include "common.h"
#include "irq.h"
#include "kernel.h"
#include "log.h"
#include "mailbox.h"
#include "utils.h"
#include "video.h"

UartSettings settings = {
  .uart = UART0,
  .tx = 14,
  .rx = 15,
};

// Screen settings
#define WIDTH 1920
#define HEIGHT 1080
#define BITS_PER_PIXEL 32
#define PADDING 50

// For the bricks
#define ROWS 5
#define COLS 7
#define BRICK_WIDTH 300
#define BRICK_HEIGHT 50

#define PADDLE_HEIGHT 10
#define PADDLE_WIDTH 150
#define PADDLE_MOVE_SPEED 25

// Gameplay
#define NUM_LIVES 3
#define INITIAL_X_SPEED 2
#define INITIAL_Y_SPEED 10

// Objects
#define SAFE_ADDRESS 0x0021000  // To store objects buffer
typedef enum { OBJ_NONE = 0, OBJ_BRICK = 1, OBJ_PADDLE = 2, OBJ_BALL = 3 } ObjectTypes;

typedef struct {
  ObjectTypes type;
  u32 x;
  u32 y;
  u32 width;
  u32 height;
  u32 color;
  u8 alive;
} Object;

u32 bricks = ROWS * COLS;
u32 numobjs = 0;
Object *objects = (Object *)SAFE_ADDRESS;
Object *ball;
Object *paddle;
unsigned char ch = 0;

char scoreboard_buffer[50];

void init_bricks() {
  u32 brick_width = BRICK_WIDTH;
  u32 brick_height = BRICK_HEIGHT;
  u32 brick_spacing_x = 10;
  u32 brick_spacing_y = 10;
  static u32 brick_colors[] = { 0xAA0000, 0x00AA00, 0x00AAAA, 0xAAAA00, 0XAA00AA, 0x0000AA };

  u32 ybrick = PADDING;
  u32 xbrick_start = PADDING;
  u32 xbrick;

  for (u32 i = 0; i < ROWS; i++) {
    xbrick = xbrick_start;
    for (u32 j = 0; j < COLS; j++) {
      video_draw_rectangle(xbrick, ybrick, brick_width, brick_height, brick_colors[i % 4]);

      objects[numobjs].type = OBJ_BRICK;
      objects[numobjs].x = xbrick;
      objects[numobjs].y = ybrick;
      objects[numobjs].width = brick_width;
      objects[numobjs].height = brick_height;
      objects[numobjs].color = brick_colors[i % 4];
      objects[numobjs].alive = 1;
      numobjs++;
      xbrick += brick_width + brick_spacing_x;
    }
    ybrick += brick_height + brick_spacing_y;
  }
}

void init_ball() {
  u32 ballradius = 5;

  video_draw_sphere(WIDTH / 2, HEIGHT / 2, ballradius, 0xFFFFFF);

  objects[numobjs].type = OBJ_BALL;
  objects[numobjs].x = (WIDTH / 2) - ballradius;   // Turn into square for later movement
  objects[numobjs].y = (HEIGHT / 2) - ballradius;  // Turn into square for later movement
  objects[numobjs].width = ballradius * 2;
  objects[numobjs].height = ballradius * 2;
  objects[numobjs].alive = 1;
  ball = &objects[numobjs];
  numobjs++;
}

void init_paddle() {
  u32 paddle_width = PADDLE_WIDTH;
  u32 paddle_height = PADDLE_HEIGHT;

  video_draw_rectangle((WIDTH - paddle_width) / 2, HEIGHT - PADDING - paddle_height, paddle_width, paddle_height, 0x550055);

  objects[numobjs].type = OBJ_PADDLE;
  objects[numobjs].x = (WIDTH - paddle_width) / 2;
  objects[numobjs].y = (HEIGHT - PADDING - paddle_height);
  objects[numobjs].width = paddle_width;
  objects[numobjs].height = paddle_height;
  objects[numobjs].alive = 1;
  paddle = &objects[numobjs];
  numobjs++;
}

void remove_object(Object *object) {
  video_draw_rectangle(object->x, object->y, object->width, object->height, BACK_COLOR);
  object->alive = 0;
}

void move_object(Object *object, u32 xoff, u32 yoff) {
  video_draw_rectangle(object->x, object->y, object->width, object->height, BACK_COLOR);
  object->x = object->x + xoff;
  object->y = object->y + yoff;
  video_draw_rectangle(object->x, object->y, object->width, object->height, object->color);
}

Object *detect_collision(Object *with, int xoff, int yoff) {
  for (u32 i = 0; i < numobjs; i++) {
    if (&objects[i] != with && objects[i].alive == 1) {
      // Check for overlap
      if (with->x + xoff < objects[i].x + objects[i].width && with->x + xoff + with->width > objects[i].x &&
          with->y + yoff < objects[i].y + objects[i].height && with->y + yoff + with->height > objects[i].y) {
        return &objects[i];
      }
    }
  }
  return 0;
}

void draw_scoreboard(int score, int lives) {
  int tens = score / 10;
  int ones = score % 10;

  log_sprint(scoreboard_buffer, "Score: %d%d     Lives: %d", tens, ones, lives);

  video_draw_str(scoreboard_buffer, (WIDTH / 2), PADDING - 25);
}

char get_uart() {
  char ch = 0;

  if (uart_read_ready()) {
    ch = uart_receive();
  }
  return ch;
}

void handle_uart0_irq() {
  while (!(settings.uart->fr & (1 << 4))) {
    ch = settings.uart->dr & 0xFF;
  }
}

void kernel_init(int points, int lives) {
  uart_init(&settings);
  video_init();
  video_set_dma(true);

  video_set_resolution(WIDTH, HEIGHT, BITS_PER_PIXEL);
  init_bricks();
  init_ball();
  init_paddle();
  draw_scoreboard(points, lives);

  irq_init_vectors();
  enable_interrupt_controller();
  irq_enable();

  while (!get_uart())
    ;
}

void kernel_main() {
  Object *found_object;

  int lives = NUM_LIVES;
  int points = 0;
  int velocity_x = INITIAL_X_SPEED;
  int velocity_y = INITIAL_Y_SPEED;

  kernel_init(points, lives);

  while (lives > 0 && bricks > 0) {
    if (ch == 'l') {
      if (paddle->x + paddle->width + PADDLE_MOVE_SPEED <= WIDTH - PADDING) {
        move_object(paddle, PADDLE_MOVE_SPEED, 0);
      }
      ch = 0;
    }
    if (ch == 'r') {
      if (paddle->x >= PADDING + PADDLE_MOVE_SPEED) {
        move_object(paddle, -PADDLE_MOVE_SPEED, 0);
      }
      ch = 0;
    }

    // Are we going to hit anything?
    found_object = detect_collision(ball, velocity_x, velocity_y);

    if (found_object) {
      if (found_object == paddle) {
        // Bounce
        velocity_y = -velocity_y;
        // If we hit the side of the paddle
        velocity_x = (ball->x + (ball->width / 2)) - (paddle->x + (paddle->width / 2));

        if (velocity_x > INITIAL_X_SPEED) {
          velocity_x = INITIAL_X_SPEED;
        } else if (velocity_x < -INITIAL_X_SPEED) {
          velocity_x = -INITIAL_X_SPEED;
        }

      } else if (found_object->type == OBJ_BRICK) {
        remove_object(found_object);

        if ((ball->x + ball->width <= found_object->x && velocity_x > 0) ||          // Hitting from left side
            (ball->x >= found_object->x + found_object->width && velocity_x < 0)) {  // Hitting from right side
          velocity_x = -velocity_x;
        } else {
          velocity_y = -velocity_y;  // Reverse Y velocity
        }

        bricks--;
        points++;
        draw_scoreboard(points, lives);
      }
    }

    move_object(ball, velocity_x, velocity_y);

    // Check we're in the game arena still
    if (ball->x + ball->width >= WIDTH - PADDING) {
      velocity_x = -INITIAL_X_SPEED;
    } else if (ball->x <= PADDING) {
      velocity_x = INITIAL_X_SPEED;
    } else if (ball->y <= PADDING) {
      velocity_y = INITIAL_Y_SPEED;
    } else if (ball->y + ball->height >= HEIGHT - PADDING) {
      lives--;

      remove_object(ball);
      remove_object(paddle);

      init_ball();
      init_paddle();
      draw_scoreboard(points, lives);
    }
  }

  if (bricks == 0) {
    video_draw_str("Well done!", (WIDTH / 2), (HEIGHT / 2));
  } else {
    remove_object(ball);
    video_draw_str("Game over!", (WIDTH / 2), (HEIGHT / 2));
  }
  video_dma();

  while (1) {
  }
}