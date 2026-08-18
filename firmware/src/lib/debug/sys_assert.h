#ifndef _SYS_ASSERT_H_
#define _SYS_ASSERT_H_

#include <stdio.h>
#include <hal/stdio_driver.h>

/**
 * @brief Assertion macro that checks an expression and calls the assertion handler if the expression is false.
 */
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

/**
 * @brief Like SYS_ASSERT, but with a defined release-build fallback instead of no-op.
 *
 * In a debug build, behaves exactly like SYS_ASSERT(expr) -- fails fast on a broken invariant.
 * In a release (NDEBUG) build, SYS_ASSERT is compiled out, so this instead runs `onFail` (e.g. a
 * `return`, a clamp, a safe default assignment) whenever expr is false, so the invariant still
 * holds afterward instead of being silently unchecked. Use this instead of a bare SYS_ASSERT
 * wherever violating the check would be memory-unsafe or state-corrupting in production, not just
 * a development-time bug to catch.
 */
#ifndef NDEBUG
#define SYS_CHECK(expr, onFail) SYS_ASSERT(expr)
#define SYS_CHECK_MSG(expr, onFail, ...) SYS_ASSERT_MSG(expr, __VA_ARGS__)
#else
#define SYS_CHECK(expr, onFail) \
    do                          \
    {                           \
        if (!(expr))            \
        {                       \
            onFail;             \
        }                       \
    } while (0)
#define SYS_CHECK_MSG(expr, onFail, ...) \
    do                                   \
    {                                    \
        if (!(expr))                     \
        {                                \
            onFail;                      \
        }                                \
    } while (0)
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