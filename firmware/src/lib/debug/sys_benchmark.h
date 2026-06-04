#ifndef _SYS_BENCHMARK_H_
#define _SYS_BENCHMARK_H_

#include <stdio.h>
#include <stdint.h>
#include <hal/time_driver.h>
#include <hal/stdio_driver.h>

#define SYS_BENCHMARK_SCOPE(expr)                                                     \
    do                                                                                \
    {                                                                                 \
        uint32_t start_internal = hal_time_get_us_since_boot();                       \
        expr                                                                          \
        uint32_t end_internal = hal_time_get_us_since_boot();                         \
        hal_stdio_printf("Benchmark result: %u us\n", end_internal - start_internal); \
    } while (0)

#define SYS_BENCHMARK_AVERAGE(name, rate, expr)                                     \
    do                                                                              \
    {                                                                               \
        static sys_benchmark_stats_t stats_internal = {};                           \
        uint32_t start_internal = hal_time_get_us_since_boot();                     \
        expr                                                                        \
        uint32_t end_internal = hal_time_get_us_since_boot();                       \
        sys_benchmark_stats_update(&stats_internal, end_internal - start_internal); \
        if (stats_internal.count >= rate)                                           \
        {                                                                           \
            sys_benchmark_stats_print(&stats_internal, #name);                      \
            sys_benchmark_stats_reset(&stats_internal);                             \
        }                                                                           \
    } while (0)

#ifdef __cplusplus
extern "C"
{
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
} sys_benchmark_stats_t;

/**
 * @brief Reset benchmark statistics
 *
 * @param stats Pointer to the benchmark statistics structure to reset
 */
void sys_benchmark_stats_reset(sys_benchmark_stats_t *stats);

/**
 * @brief Update benchmark statistics with a new sample
 *
 * @param stats Pointer to the benchmark statistics structure to update
 * @param ticks The new sample in microseconds to add to the statistics
 */
void sys_benchmark_stats_update(sys_benchmark_stats_t *stats, uint32_t ticks);

/**
 * @brief Calculate the variance of the benchmark statistics
 *
 * @param stats Pointer to the benchmark statistics structure
 * @return The variance of the benchmark samples
 */
double sys_benchmark_stats_variance(const sys_benchmark_stats_t *stats);

/**
 * @brief Calculate the standard deviation of the benchmark statistics
 *
 * @param stats Pointer to the benchmark statistics structure
 * @return The standard deviation of the benchmark samples
 */
double sys_benchmark_stats_stddev(const sys_benchmark_stats_t *stats);

/**
 * @brief Print the benchmark statistics to the console
 *
 * @param stats Pointer to the benchmark statistics structure to print
 * @param name Name of the benchmark for labeling the output
 */
void sys_benchmark_stats_print(const sys_benchmark_stats_t *stats, const char *name);

#ifdef __cplusplus
}
#endif

#endif