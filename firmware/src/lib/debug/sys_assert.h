#ifndef _SYS_ASSERT_H_
#define _SYS_ASSERT_H_

#include <stdio.h>
#include <hal/stdio_driver.h>

#ifndef NDEBUG

#define SYS_ASSERT(expr)                                     \
    do                                                       \
    {                                                        \
        if (!(expr))                                         \
        {                                                    \
            sys_assert_handler(#expr, __FILE__, __LINE__); \
        }                                                    \
    } while (0)

#define SYS_ASSERT_MSG(expr, ...)                            \
    do                                                       \
    {                                                        \
        if (!(expr))                                         \
        {                                                    \
            hal_stdio_printf("\t");                          \
            hal_stdio_printf(__VA_ARGS__);                   \
            hal_stdio_printf("\n");                          \
            sys_assert_handler(#expr, __FILE__, __LINE__); \
        }                                                    \
    } while (0)

#else

#define SYS_ASSERT(expr) ((void)sizeof(expr))
#define SYS_ASSERT_MSG(expr, ...) ((void)sizeof(expr))

#endif

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Assertion handler function prototype. DO NOT CALL THIS FUNCTION DIRECTLY. Use the SYS_ASSERT macro instead.
 *
 * @param expr The expression that failed the assertion.
 * @param file The name of the source file where the assertion failed.
 * @param line The line number in the source file where the assertion failed.
 */
void sys_assert_handler(const char *expr, const char *file, int line);

#ifdef __cplusplus
}
#endif

#endif