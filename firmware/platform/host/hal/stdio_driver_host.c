#include "hal/stdio_driver.h"
#include <stdarg.h>
#include <stdio.h>

void hal_stdio_init(void)
{
    (void)0;
}

bool hal_stdio_is_usb_connected(void)
{
    return true;
}

bool hal_stdio_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    int result = vprintf(fmt, args);

    va_end(args);

    return result >= 0;
}

bool hal_stdio_send_buffer(const uint8_t *buffer, size_t len)
{
    size_t result = fwrite(buffer, 1, len, stdout);
    fflush(stdout);

    return result == len;
}

bool hal_stdio_read_byte(uint8_t *byte)
{
    (void)byte;

    return false;
}