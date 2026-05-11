#include "sys_log.h"
#include <hal/stdio_driver.h>
#include <stdio.h>
#include <stdarg.h>

void __sys_log(const char *level, const char *additional, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    char buffer[256];
    char str[64];

    snprintf(str, sizeof(str), "[%s] (%s)", level, additional);
    int len = snprintf(buffer, sizeof(buffer), "%-24s ", str);

    if (len >= 0 && len < (int)sizeof(buffer))
    {
        vsnprintf(buffer + len, sizeof(buffer) - len, fmt, args);
    }

    hal_stdio_printf("%s\n", buffer);

    va_end(args);
}