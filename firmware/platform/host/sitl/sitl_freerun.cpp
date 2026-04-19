#include "sitl.h"
#include "osal/task.h"
#include "osal/systime.h"
#include "hal/time_driver.h"
#include <chrono>
#include <thread>

static std::chrono::steady_clock::time_point g_startTime;

void sitl_init_godmode(void)
{
    (void)0;
}

void sitl_time_tick(void)
{
    (void)0;
}

void sitl_wait_for_threads_ready(void)
{
    (void)0;
}

void sitl_stop(void)
{
    (void)0;
}

void osal_task_system_init(void)
{
    (void)0;
}

void osal_task_create(const char *task_name, osal_task_function_t task_func, void *arg, uint8_t *stack_buffer, size_t stack_size, osal_task_priority_t priority)
{
    (void)task_name;
    (void)stack_buffer;
    (void)stack_size;
    (void)priority;

    std::thread(task_func, arg).detach();
}

void osal_task_start_scheduler(void)
{
    while (osal_task_should_run())
    {
        osal_task_delay_ms(100);
    }
}

bool osal_task_should_run(void)
{
    return true;
}

uint32_t osal_systime_get_ms(void)
{
    return hal_time_get_ms_since_boot();
}

void osal_task_delay_ms(uint32_t ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

bool osal_task_delay_until(uint32_t *last_wake_time, uint32_t period_ms)
{
    *last_wake_time += period_ms;

    osal_task_delay_ms(period_ms);

    return true;
}

void hal_time_init(void)
{
    g_startTime = std::chrono::steady_clock::now();
}

uint32_t hal_time_get_ms_since_boot(void)
{
    auto now = std::chrono::steady_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_startTime);
    return static_cast<uint32_t>(diff.count());
}

uint32_t hal_time_get_us_since_boot(void)
{
    auto now = std::chrono::steady_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::microseconds>(now - g_startTime);
    return static_cast<uint32_t>(diff.count());
}

void hal_time_sleep_ms(uint32_t ms)
{
    (void)ms;
}

void hal_time_sleep_us(uint32_t us)
{
    (void)us;
}