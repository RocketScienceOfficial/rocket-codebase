#include "sys_assert.h"
#include <stdlib.h>

void sys_assert_handler(const char *expr, const char *file, int line)
{
    hal_stdio_printf("\tASSERTION FAILED: %s, file: %s:%d\n", expr, file, line);

    abort();
}