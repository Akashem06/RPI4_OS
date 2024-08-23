#include "log.h"

void log_format(const char *format, va_list args) {
    while (*format) {
        if (*format == '%') {
            format++;
            switch (*format) {
                case 'd': {
                    int value = va_arg(args, int);
                    char buffer[12];
                    int i = 0;
                    u8 is_negative = 0;

                    if (value < 0) {
                        is_negative = 1;
                        value = -value;
                    }

                    do {
                        buffer[i++] = (value % 10) + '0';
                        value /= 10;
                    } while (value > 0);

                    if (is_negative) {
                        buffer[i++] = '-';
                    }

                    while (--i >= 0) {
                        uart_transmit(buffer[i]);
                    }
                    break;
                }
                case 's': {
                    char *str = va_arg(args, char*);
                    while (*str) {
                        uart_transmit(*str);
                        str++;
                    }
                    break;
                }
                default:
                    uart_transmit('%');
                    uart_transmit(*format);
                    break;
            }

        } else {
            uart_transmit(*format);
        }

        format++;
    }
}

void log(char *format, ...) {
    va_list args;
    va_start(args, format);
    log_format(format, args);
    va_end(args);
}