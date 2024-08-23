#pragma once

#include "common.h"
#include "peripherals/base.h"
#include "peripherals/aux_reg.h"

#define TX_PIN 14
#define RX_PIN 15

void mini_uart_init();
char mini_uart_receive();
void mini_uart_transmit(char c);
void mini_uart_transmit_string(char *message);