#pragma once

#include "common.h"
#include "peripherals/base.h"
#include "peripherals/aux_reg.h"

#define TX_PIN 14
#define RX_PIN 15

void uart_init();
char uart_receive();
void uart_transmit(char c);
void uart_transmit_string(char *message);