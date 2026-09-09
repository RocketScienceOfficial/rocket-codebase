#include "osal/task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MAX_TASKS 16

static StaticTask_t g_tasks[MAX_TASKS];
static uint8_t g_task_count = 0;

void osal_task_create(const char *task_name, osal_task_function_t task_func, void *arg, uint8_t *stack_buffer, size_t stack_size, osal_task_priority_t priority)
{
    UBaseType_t rtos_priority;

    switch (priority)
    {
    case OSAL_TASK_PRIORITY_LOW:
        rtos_priority = tskIDLE_PRIORITY + 1;
        break;
    case OSAL_TASK_PRIORITY_NORMAL:
        rtos_priority = tskIDLE_PRIORITY + 2;
        break;
    case OSAL_TASK_PRIORITY_HIGH:
        rtos_priority = configMAX_PRIORITIES - 1;
        break;
    default:
        rtos_priority = tskIDLE_PRIORITY + 1;
        break;
    }

    uint16_t depth = (uint16_t)(stack_size / sizeof(StackType_t));

    xTaskCreateStatic(task_func, task_name, depth, arg, rtos_priority, (StackType_t *const)stack_buffer, &g_tasks[g_task_count++]);
}

void osal_task_start_scheduler(void)
{
}

bool osal_task_should_run(void)
{
    return true;
}

uint32_t osal_task_get_ms(void)
{
    return pdTICKS_TO_MS(xTaskGetTickCount());
}

void osal_task_delay_ms(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

bool osal_task_delay_until(uint32_t deadline_ms)
{
    TickType_t anchor = xTaskGetTickCount();

    // Signed difference so the comparison survives the millisecond counter wrap.
    const int32_t remaining_ms = (int32_t)(deadline_ms - pdTICKS_TO_MS(anchor));

    if (remaining_ms <= 0)
    {
        taskYIELD();

        return false;
    }

    TickType_t increment = pdMS_TO_TICKS((uint32_t)remaining_ms);

    // xTaskDelayUntil asserts on a zero increment, and one tick is the finest wait the scheduler
    // can express anyway.
    if (increment == 0)
    {
        increment = 1;
    }

    xTaskDelayUntil(&anchor, increment);

    return true;
}
