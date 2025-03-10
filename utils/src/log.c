#include "log.h"

LogModes config_mode = LOG_MODE_UART;

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
            if (config_mode == LOG_MODE_UART) {
              uart_transmit(buffer[i]);
            } else if (config_mode == LOG_MODE_MINIUART) {
              mini_uart_transmit(buffer[i]);
            }
          }
          break;
        }
        case 'l': {
          format++;
          long value = va_arg(args, long);
          char buffer[20];
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
            if (config_mode == LOG_MODE_UART) {
              uart_transmit(buffer[i]);
            } else if (config_mode == LOG_MODE_MINIUART) {
              mini_uart_transmit(buffer[i]);
            }
          }
          break;
        }

        case 's': {
          char *str = va_arg(args, char *);
          while (*str) {
            if (config_mode == LOG_MODE_UART) {
              uart_transmit(*str);
            } else if (config_mode == LOG_MODE_MINIUART) {
              mini_uart_transmit(*str);
            }
            str++;
          }
          break;
        }
        default:
          if (config_mode == LOG_MODE_UART) {
            uart_transmit('%');
            uart_transmit(*format);
          } else if (config_mode == LOG_MODE_MINIUART) {
            mini_uart_transmit('%');
            mini_uart_transmit(*format);
          }
          break;
      }

    } else {
      if (config_mode == LOG_MODE_UART) {
        uart_transmit(*format);
      } else if (config_mode == LOG_MODE_MINIUART) {
        mini_uart_transmit(*format);
      }
    }

    format++;
  }
}

int log_sprint(char *buffer, const char *format, ...) {
  va_list args;
  va_start(args, format);

  char *buf_ptr = buffer;
  const char *fmt_ptr = format;
  int written = 0;

  while (*fmt_ptr) {
    if (*fmt_ptr == '%') {
      fmt_ptr++;
      switch (*fmt_ptr) {
        case 's': {
          char *str = va_arg(args, char *);
          while (*str) {
            *buf_ptr++ = *str++;
            written++;
          }
          break;
        }
        case 'd': {
          int num = va_arg(args, int);
          char temp[10];
          int i = 0;

          if (num < 0) {
            *buf_ptr++ = '-';
            written++;
            num = -num;
          }

          if (num == 0) {
            *buf_ptr++ = '0';
            written++;
          } else {
            while (num) {
              temp[i++] = (num % 10) + '0';
              num /= 10;
            }

            while (i--) {
              *buf_ptr++ = temp[i];
              written++;
            }
          }
          break;
        }
        case 'l': {
          fmt_ptr++;
          if (*fmt_ptr == 'd') {
            long num = va_arg(args, long);
            char temp[20];
            int i = 0;

            if (num < 0) {
              *buf_ptr++ = '-';
              written++;
              num = -num;
            }

            if (num == 0) {
              *buf_ptr++ = '0';
              written++;
            } else {
              while (num) {
                temp[i++] = (num % 10) + '0';
                num /= 10;
              }

              while (i--) {
                *buf_ptr++ = temp[i];
                written++;
              }
            }
          }
          break;
        }
        case 'c': {
          char c = (char)va_arg(args, int);
          *buf_ptr++ = c;
          written++;
          break;
        }
        default: {
          *buf_ptr++ = '%';
          *buf_ptr++ = *fmt_ptr;
          written += 2;
          break;
        }
      }
    } else {
      *buf_ptr++ = *fmt_ptr;
      written++;
    }
    fmt_ptr++;
  }

  *buf_ptr = '\0';
  va_end(args);

  return written;
}

void log(char *format, ...) {
  va_list args;
  va_start(args, format);
  log_format(format, args);
  va_end(args);
}

void log_init(LogModes mode) {
  config_mode = mode;
}