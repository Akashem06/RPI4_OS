#include <stdarg.h>
#include "uart.h"
#include "common.h"

// https://hackernoon.com/what-is-va_list-in-c-exploring-the-secrets-of-ft_printf

void log_format(const char *format, va_list args);

void log(char *format, ...);
int log_sprint(char *buffer, const char *format, ...);
