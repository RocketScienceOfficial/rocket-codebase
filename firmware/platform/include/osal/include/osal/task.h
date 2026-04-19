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
 * @brief Initialize the task system. This function must be called before creating any tasks.
 */
void osal_task_system_init(void);

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

#ifdef __cplusplus
}
#endif

#endif