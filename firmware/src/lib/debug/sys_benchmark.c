#include "sys_benchmark.h"
#include <math.h>

void sys_benchmark_stats_reset(sys_benchmark_stats_t *stats)
{
    stats->count = 0;
    stats->min_ticks = UINT32_MAX;
    stats->max_ticks = 0;
    stats->mean = 0.0;
    stats->M2 = 0.0;
}

void sys_benchmark_stats_update(sys_benchmark_stats_t *stats, uint32_t ticks)
{
    stats->count++;

    if (ticks < stats->min_ticks)
    {
        stats->min_ticks = ticks;
    }
    if (ticks > stats->max_ticks)
    {
        stats->max_ticks = ticks;
    }

    // Welford's algorithm
    double delta = (double)ticks - stats->mean;
    stats->mean += delta / stats->count;
    double delta2 = (double)ticks - stats->mean;
    stats->M2 += delta * delta2;
}

double sys_benchmark_stats_variance(const sys_benchmark_stats_t *stats)
{
    if (stats->count < 2)
    {
        return 0.0;
    }

    return stats->M2 / (stats->count - 1);
}

double sys_benchmark_stats_stddev(const sys_benchmark_stats_t *stats)
{
    return sqrt(sys_benchmark_stats_variance(stats));
}

void sys_benchmark_stats_print(const sys_benchmark_stats_t *stats, const char *name)
{
    hal_stdio_printf("--- %s ---\n", name);
    hal_stdio_printf("Samples : %lu\n", (unsigned long)stats->count);
    hal_stdio_printf("Min     : %lu us\n", (unsigned long)(stats->count > 0 ? stats->min_ticks : 0));
    hal_stdio_printf("Max     : %lu us\n", (unsigned long)stats->max_ticks);
    hal_stdio_printf("Mean    : %.2f us\n", stats->mean);
    hal_stdio_printf("StdDev  : %.2f us\n", sys_benchmark_stats_stddev(stats));
    hal_stdio_printf("-------------------\n");
}