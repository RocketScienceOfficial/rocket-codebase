#include "hal/stdio_driver.h"
#include "pico/stdlib.h"
#include "pico/error.h"
#include "pico/stdio_usb.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void hal_stdio_init(void)
{
    stdio_init_all();
    stdio_set_translate_crlf(&stdio_usb, false);
}

bool hal_stdio_is_usb_connected(void)
{
    return stdio_usb_connected();
}

void hal_stdio_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    vprintf(fmt, args);

    va_end(args);

    if (fmt[0] != '\0' && fmt[strlen(fmt) - 1] == '\n')
    {
        putchar('\r');
    }
}

void hal_stdio_send_buffer(const uint8_t *buffer, size_t len)
{
    fwrite(buffer, 1, len, stdout);
    fflush(stdout);
}

bool hal_stdio_read_byte(uint8_t *byte)
{
    int res = getchar_timeout_us(0);

    if (res < 0)
    {
        return false;
    }
    else
    {
        *byte = (uint8_t)res;

        return true;
    }
}