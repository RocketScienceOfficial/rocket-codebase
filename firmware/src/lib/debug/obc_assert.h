#ifndef _OBC_ASSERT_H_
#define _OBC_ASSERT_H_

#include <stdio.h>
#include <hal/stdio_driver.h>

#ifndef NDEBUG

#define OBC_ASSERT(expr)                                     \
    do                                                       \
    {                                                        \
        if (!(expr))                                         \
        {                                                    \
            __obc_assert_handler(#expr, __FILE__, __LINE__); \
        }                                                    \
    } while (0)

#define OBC_ASSERT_MSG(expr, ...)                            \
    do                                                       \
    {                                                        \
        if (!(expr))                                         \
        {                                                    \
            hal_stdio_printf("\t");                          \
            hal_stdio_printf(__VA_ARGS__);                   \
            hal_stdio_printf("\n");                          \
            __obc_assert_handler(#expr, __FILE__, __LINE__); \
        }                                                    \
    } while (0)

#else

#define OBC_ASSERT(expr) ((void)0)
#define OBC_ASSERT_MSG(expr, ...) ((void)0)

#endif

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Assertion handler function prototype. DO NOT CALL THIS FUNCTION DIRECTLY. Use the OBC_ASSERT macro instead.
 *
 * @param expr The expression that failed the assertion.
 * @param file The name of the source file where the assertion failed.
 * @param line The line number in the source file where the assertion failed.
 */
void __obc_assert_handler(const char *expr, const char *file, int line);

#ifdef __cplusplus
}
#endif

#endif