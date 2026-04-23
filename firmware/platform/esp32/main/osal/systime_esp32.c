#include "osal/systime.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

uint32_t osal_systime_get_ms(void)
{
    return pdTICKS_TO_MS(xTaskGetTickCount());
}

void osal_task_delay_ms(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

bool osal_task_delay_until(uint32_t *last_wake_time, uint32_t period_ms)
{
    TickType_t ticks = pdMS_TO_TICKS(period_ms);
    BaseType_t was_delayed = xTaskDelayUntil((TickType_t *)last_wake_time, ticks);

    return (was_delayed == pdTRUE);
}