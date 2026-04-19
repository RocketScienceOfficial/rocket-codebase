#ifndef _SYSTIME_H_
#define _SYSTIME_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get the current time in milliseconds since the task started.
 * 
 * @return Current time in milliseconds.
 */
uint32_t osal_systime_get_ms(void);

/**
 * @brief Delay the current task for a specified number of milliseconds.
 * 
 * @param ms Number of milliseconds to delay.
 */
void osal_task_delay_ms(uint32_t ms);

/**
 * @brief Delay the current task until a specified target time in milliseconds.
 * 
 * @param last_wake_time Pointer to the last wake time.
 * @param period_ms Period in milliseconds.
 * @return true if the task was delayed, false if the target time has already passed.
 */
bool osal_task_delay_until(uint32_t *last_wake_time, uint32_t period_ms);

#ifdef __cplusplus
}
#endif

#endif