#ifndef _TIME_DRIVER_H
#define _TIME_DRIVER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the time driver
 */
void hal_time_init(void);

/**
 * @brief Gets the current time in milliseconds
 *
 * @return Current time in milliseconds
 */
uint32_t hal_time_get_ms_since_boot(void);

/**
 * @brief Gets the current time in microseconds
 *
 * @return Current time in microseconds
 */
uint32_t hal_time_get_us_since_boot(void);

/**
 * @brief Sleep for a specified number of milliseconds
 * 
 * @param ms Number of milliseconds to sleep
 */
void hal_time_sleep_ms(uint32_t ms);

/**
 * @brief Sleep for a specified number of microseconds
 * 
 * @param us Number of microseconds to sleep
 */
void hal_time_sleep_us(uint32_t us);

#ifdef __cplusplus
}
#endif

#endif