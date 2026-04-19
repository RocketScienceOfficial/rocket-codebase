#ifndef _OBC_BENCHMARK_H_
#define _OBC_BENCHMARK_H_

#include <stdio.h>
#include <stdint.h>
#include <hal/time_driver.h>
#include <hal/stdio_driver.h>

#define OBC_BENCHMARK_SCOPE(expr)                                       \
    do                                                                  \
    {                                                                   \
        uint32_t __start = hal_time_get_us_since_boot();                \
        expr                                                            \
        uint32_t __end = hal_time_get_us_since_boot();                  \
        hal_stdio_printf("Benchmark result: %u us\n", __end - __start); \
    } while (0)

#define OBC_BENCHMARK_AVERAGE(name, rate, expr)                \
    do                                                         \
    {                                                          \
        static obc_benchmark_stats_t __stats = {};             \
        uint32_t __start = hal_time_get_us_since_boot();       \
        expr                                                   \
        uint32_t __end = hal_time_get_us_since_boot();         \
        __obc_benchmark_stats_update(&__stats, __end - __start); \
        if (__stats.count >= rate)                             \
        {                                                      \
            __obc_benchmark_stats_print(&__stats, #name);      \
            __obc_benchmark_stats_reset(&__stats);             \
        }                                                      \
    } while (0)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Structure to hold benchmark statistics
 */
typedef struct
{
    uint32_t count;
    uint32_t min_ticks;
    uint32_t max_ticks;
    double mean;
    double M2;
} obc_benchmark_stats_t;

/**
 * @brief Reset benchmark statistics
 * 
 * @param stats Pointer to the benchmark statistics structure to reset
 */
void __obc_benchmark_stats_reset(obc_benchmark_stats_t *stats);

/**
 * @brief Update benchmark statistics with a new sample
 * 
 * @param stats Pointer to the benchmark statistics structure to update
 * @param ticks The new sample in microseconds to add to the statistics
 */
void __obc_benchmark_stats_update(obc_benchmark_stats_t *stats, uint32_t ticks);

/**
 * @brief Calculate the variance of the benchmark statistics
 * 
 * @param stats Pointer to the benchmark statistics structure
 * @return The variance of the benchmark samples
 */
double __obc_benchmark_stats_variance(const obc_benchmark_stats_t *stats);

/**
 * @brief Calculate the standard deviation of the benchmark statistics
 * 
 * @param stats Pointer to the benchmark statistics structure
 * @return The standard deviation of the benchmark samples
 */
double __obc_benchmark_stats_stddev(const obc_benchmark_stats_t *stats);

/**
 * @brief Print the benchmark statistics to the console
 * 
 * @param stats Pointer to the benchmark statistics structure to print
 * @param name Name of the benchmark for labeling the output
 */
void __obc_benchmark_stats_print(const obc_benchmark_stats_t *stats, const char *name);

#ifdef __cplusplus
}
#endif


#endif