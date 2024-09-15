#include "kernel.h"
#include "common.h"
#include "gpio.h"
#include "log.h"
#include "utils.h"
#include "irq.h"
#include "uart.h"
#include "irq.h"
#include "timer.h"
#include "mailbox.h"
#include "video.h"

UartSettings settings = {
    .uart = UART0,
    .tx = 14,
    .rx = 15,
};

u8 led_flag = 0;

void timer_callback() {
    if (led_flag) {
        gpio_set_low(17);
        led_flag = 0;
    } else {
        gpio_set_high(17);
        led_flag = 1;
    }
}

void kernel_main() {
    uart_init(&settings);
    timer_init(1, CLOCK_HZ, timer_callback);
    gpio_set_function(17, GF_OUTPUT);

    irq_init_vectors();
    enable_interrupt_controller();
    irq_enable();
    
    log("CORE CLOCK: %d\r\n", mailbox_clock_rate(CT_CORE));
    delay(1000000);
    log("EMMC CLOCK: %d\r\n", mailbox_clock_rate(CT_EMMC));
    delay(1000000);
    log("UART CLOCK: %d\r\n", mailbox_clock_rate(CT_UART));
    delay(1000000);
    log("ARM  CLOCK: %d\r\n", mailbox_clock_rate(CT_ARM));
    delay(1000000);

    for (int i=0; i<3; i++) {
        bool on = mailbox_power_check(i);

        log("POWER DOMAIN STATUS FOR %d = %d\r\n", i, on);
    }

    for (int i=0; i<3; i++) {
        u32 on = 1;
        mailbox_generic_command(RPI_FIRMWARE_SET_DOMAIN_STATE, i, &on);

        log("SET POWER DOMAIN STATUS FOR %d = %d\r\n", i, on);
    }

    for (int i=0; i<3; i++) {
        bool on = mailbox_power_check(i);

        log("POWER DOMAIN STATUS FOR %d = %d\r\n", i, on);
    }

    int el = get_el();

    video_init();
    video_set_dma(true);
    log("Resolution 1900x1200\n");
    video_set_resolution(1920, 1080, 32);

    video_draw_rectangle(500, 500, 200, 200, 0xFF2257FF);
    video_draw_sphere(960, 1030, 25, 0xFFFF5733);

    video_draw_str("Hello world!", 100, 100);
    video_draw_char('O', 50, 50);

    while (1) {

        u32 max_temp = 0;

        mailbox_generic_command(RPI_FIRMWARE_GET_MAX_TEMPERATURE, 0, &max_temp);

        u32 cur_temp = 0;

        mailbox_generic_command(RPI_FIRMWARE_GET_TEMPERATURE, 0, &cur_temp);

        log("Cur temp: %dC MAX: %dC\r\n", cur_temp / 1000, max_temp / 1000);
        log("Hello World! EL LEVEL: %d\r\n", el);

        delay(2000000);
    }
}