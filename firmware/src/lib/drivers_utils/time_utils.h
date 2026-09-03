#ifndef _TIME_UTILS_H
#define _TIME_UTILS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Delays the execution of the current task for a specified number of microseconds using the OSAL task delay function.
 * 
 * @param delayUs The number of microseconds to delay the task. The function will round up to the nearest millisecond.
 */
void time_utils_delay_us_osal(uint32_t delayUs);

#ifdef __cplusplus
}
#endif

#endif