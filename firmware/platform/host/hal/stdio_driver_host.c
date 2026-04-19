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

void hal_stdio_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    vprintf(fmt, args);
    
    va_end(args);
}

void hal_stdio_send_buffer(const uint8_t *buffer, size_t len)
{
    fwrite(buffer, 1, len, stdout);
    fflush(stdout);
}

bool hal_stdio_read_byte(uint8_t *byte)
{
    (void)byte;

    return false;
}