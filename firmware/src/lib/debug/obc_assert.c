#include "obc_assert.h"
#include <stdlib.h>

void __obc_assert_handler(const char *expr, const char *file, int line)
{
    hal_stdio_printf("\tASSERTION FAILED: %s, file: %s:%d\n", expr, file, line);

    abort();
}