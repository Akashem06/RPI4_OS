#include "log.h"

#include <stddef.h>

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
        case 'u': {
          unsigned int value = va_arg(args, unsigned int);
          char buffer[12];
          int i = 0;

          do {
            buffer[i++] = (value % 10) + '0';
            value /= 10;
          } while (value > 0);

          while (--i >= 0) {
            if (config_mode == LOG_MODE_UART) {
              uart_transmit(buffer[i]);
            } else if (config_mode == LOG_MODE_MINIUART) {
              mini_uart_transmit(buffer[i]);
            }
          }
          break;
        }
        case 'x': {
          unsigned int value = va_arg(args, unsigned int);
          char buffer[12];
          int i = 0;
          char hex_chars[] = "0123456789abcdef";

          do {
            buffer[i++] = hex_chars[value & 0xF];
            value >>= 4;
          } while (value > 0);

          while (--i >= 0) {
            if (config_mode == LOG_MODE_UART) {
              uart_transmit(buffer[i]);
            } else if (config_mode == LOG_MODE_MINIUART) {
              mini_uart_transmit(buffer[i]);
            }
          }
          break;
        }
        case 'p': {
          void *ptr = va_arg(args, void *);
          unsigned long value = (unsigned long)ptr;
          char buffer[20];
          int i = 0;
          char hex_chars[] = "0123456789abcdef";

          // Print '0x' prefix
          if (config_mode == LOG_MODE_UART) {
            uart_transmit('0');
            uart_transmit('x');
          } else if (config_mode == LOG_MODE_MINIUART) {
            mini_uart_transmit('0');
            mini_uart_transmit('x');
          }

          // If pointer is NULL, print "NULL" instead of address
          if (value == 0) {
            if (config_mode == LOG_MODE_UART) {
              uart_transmit('N');
              uart_transmit('U');
              uart_transmit('L');
              uart_transmit('L');
            } else if (config_mode == LOG_MODE_MINIUART) {
              mini_uart_transmit('N');
              mini_uart_transmit('U');
              mini_uart_transmit('L');
              mini_uart_transmit('L');
            }
            break;
          }

          // Print all 16 hex digits for 64-bit address
          for (i = 0; i < 16; i++) {
            buffer[15 - i] = hex_chars[value & 0xF];
            value >>= 4;
          }

          for (i = 0; i < 16; i++) {
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
          // Handle %lx for 64-bit hex values
          if (*format == 'x') {
            unsigned long value = va_arg(args, unsigned long);
            char buffer[20];
            int i = 0;
            char hex_chars[] = "0123456789abcdef";

            do {
              buffer[i++] = hex_chars[value & 0xF];
              value >>= 4;
            } while (value > 0);

            while (--i >= 0) {
              if (config_mode == LOG_MODE_UART) {
                uart_transmit(buffer[i]);
              } else if (config_mode == LOG_MODE_MINIUART) {
                mini_uart_transmit(buffer[i]);
              }
            }
          }
          // Original %ld implementation
          else if (*format == 'd') {
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
          }
          break;
        }
        case 's': {
          char *str = va_arg(args, char *);
          if (str == NULL) {
            // Handle NULL strings
            char *null_str = "(null)";
            while (*null_str) {
              if (config_mode == LOG_MODE_UART) {
                uart_transmit(*null_str);
              } else if (config_mode == LOG_MODE_MINIUART) {
                mini_uart_transmit(*null_str);
              }
              null_str++;
            }
          } else {
            while (*str) {
              if (config_mode == LOG_MODE_UART) {
                uart_transmit(*str);
              } else if (config_mode == LOG_MODE_MINIUART) {
                mini_uart_transmit(*str);
              }
              str++;
            }
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
          if (str == NULL) {
            char *null_str = "(null)";
            while (*null_str) {
              *buf_ptr++ = *null_str++;
              written++;
            }
          } else {
            while (*str) {
              *buf_ptr++ = *str++;
              written++;
            }
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
        case 'u': {
          unsigned int num = va_arg(args, unsigned int);
          char temp[10];
          int i = 0;

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
        case 'x': {
          unsigned int num = va_arg(args, unsigned int);
          char temp[10];
          int i = 0;
          char hex_chars[] = "0123456789abcdef";

          if (num == 0) {
            *buf_ptr++ = '0';
            written++;
          } else {
            while (num) {
              temp[i++] = hex_chars[num & 0xF];
              num >>= 4;
            }

            while (i--) {
              *buf_ptr++ = temp[i];
              written++;
            }
          }
          break;
        }
        case 'p': {
          void *ptr = va_arg(args, void *);
          unsigned long num = (unsigned long)ptr;

          // Add 0x prefix
          *buf_ptr++ = '0';
          *buf_ptr++ = 'x';
          written += 2;

          if (num == 0) {
            // Special case for NULL pointer
            char *null_str = "NULL";
            while (*null_str) {
              *buf_ptr++ = *null_str++;
              written++;
            }
          } else {
            char temp[18];  // 16 hex digits + 2 for "0x"
            int i = 0;
            char hex_chars[] = "0123456789abcdef";

            // Always print full 64-bit address
            for (i = 0; i < 16; i++) {
              temp[15 - i] = hex_chars[num & 0xF];
              num >>= 4;
            }

            for (i = 0; i < 16; i++) {
              *buf_ptr++ = temp[i];
              written++;
            }
          }
          break;
        }
        case 'l': {
          fmt_ptr++;
          if (*fmt_ptr == 'x') {
            unsigned long num = va_arg(args, unsigned long);
            char temp[20];
            int i = 0;
            char hex_chars[] = "0123456789abcdef";

            if (num == 0) {
              *buf_ptr++ = '0';
              written++;
            } else {
              while (num) {
                temp[i++] = hex_chars[num & 0xF];
                num >>= 4;
              }

              while (i--) {
                *buf_ptr++ = temp[i];
                written++;
              }
            }
          } else if (*fmt_ptr == 'd') {
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