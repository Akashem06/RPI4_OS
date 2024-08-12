#pragma once

#include "common.h"
#include "peripherals/base.h"

void uart_init();
char uart_receive();
void uart_transmit(char c);
void uart_transmit_string(char *message);