#ifndef _TASK_H_
#define _TASK_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Task priority levels.
 */
typedef enum
{
    OSAL_TASK_PRIORITY_LOW = 0,
    OSAL_TASK_PRIORITY_NORMAL = 1,
    OSAL_TASK_PRIORITY_HIGH = 2,
} osal_task_priority_t;

typedef void (*osal_task_function_t)(void *arg); // Task function type definition.

/**
 * @brief Create a new task.
 *
 * @param task_name Name of the task.
 * @param task_func Function pointer to the task function.
 * @param arg Argument to be passed to the task function.
 * @param stack_buffer Pointer to the stack buffer for the task. Must be at least as large as stack_size.
 * @param stack_size Size of the task stack in bytes.
 * @param priority Priority level of the task.
 */
void osal_task_create(const char *task_name, osal_task_function_t task_func, void *arg, uint8_t* stack_buffer, size_t stack_size, osal_task_priority_t priority);

/**
 * @brief Start the task scheduler.
 */
void osal_task_start_scheduler(void);

/**
 * @brief Check if the current task should continue running. This function can be used within a task loop to determine if the task should exit.
 *
 * @return true if the task should continue running, false if it should exit.
 */
bool osal_task_should_run(void);

/**
 * @brief Get the current system time in milliseconds since boot.
 *
 * Wraps roughly every 49.7 days. Compare timestamps as signed differences, never as absolute
 * values, so the comparison stays correct across the wrap.
 *
 * @return Current time in milliseconds.
 */
uint32_t osal_task_get_ms(void);

/**
 * @brief Delay the current task for a relative number of milliseconds.
 *
 * @warning Under the lockstep SITL platform this sleeps in real time and does NOT participate in
 * the simulation barrier, so a task blocked here stalls the whole simulation. Rate-driven task
 * loops must use osal_task_delay_until() instead. This function is for driver-level busy-waits
 * on targets where real time and system time are the same thing.
 *
 * @param ms Number of milliseconds to delay.
 */
void osal_task_delay_ms(uint32_t ms);

/**
 * @brief Block the current task until an absolute deadline on the osal_task_get_ms() clock.
 *
 * The deadline is compared as a signed difference against the current time, so it behaves
 * correctly across the millisecond counter wrap as long as it is less than ~24.8 days away.
 * If the deadline has already passed the function returns immediately without blocking, which
 * lets a caller detect that it overran its schedule.
 *
 * @note This is the only OSAL call that participates in the lockstep SITL barrier. Under that
 * platform, virtual time only advances once every worker task is blocked inside this function,
 * so a rate-driven task loop must reach it on every pass. A loop that never blocks here (because
 * its deadline is permanently in the past) will stall simulated time for every other task.
 *
 * @param deadline_ms Absolute wake time, on the same clock as osal_task_get_ms().
 * @return true if the task blocked, false if the deadline had already passed.
 */
bool osal_task_delay_until(uint32_t deadline_ms);

#ifdef __cplusplus
}
#endif

#endif
