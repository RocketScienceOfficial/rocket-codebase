#include "hal/stdio_driver.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>

void hal_stdio_init(void)
{
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);

    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);

    if (flags != -1)
    {
        fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    }
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
    int res = read(STDIN_FILENO, byte, 1);

    return res == 1;
}