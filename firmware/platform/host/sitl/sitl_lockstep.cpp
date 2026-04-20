#include "sitl.h"
#include "osal/task.h"
#include "osal/systime.h"
#include "hal/time_driver.h"
#include <atomic>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <map>

struct ThreadState
{
    uint32_t target_wake_time;
    bool is_sleeping;
};

static std::atomic<bool> g_is_running{true};
static std::atomic<uint32_t> g_current_tick;
static uint32_t g_active_workers;
static std::mutex g_mutex;
static std::condition_variable g_cv_workers;
static std::condition_variable g_cv_god;
static std::map<std::thread::id, ThreadState> g_threads;

void sitl_init_godmode(void)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    g_active_workers--;
}

void sitl_time_tick(void)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    g_current_tick.fetch_add(1, std::memory_order_relaxed);
    
    for (auto &pair : g_threads)
    {
        if (pair.second.is_sleeping && g_current_tick.load(std::memory_order_relaxed) >= pair.second.target_wake_time)
        {
            pair.second.is_sleeping = false;

            g_active_workers++; 
        }
    }

    if (g_active_workers > 0)
    {
        g_cv_workers.notify_all();
    }
}

void sitl_wait_for_threads_ready(void)
{
    std::unique_lock<std::mutex> lock(g_mutex);
    g_cv_god.wait(lock, []{ return g_active_workers == 0; });
}

void sitl_stop(void)
{
    g_is_running.store(false, std::memory_order_relaxed);

    sitl_time_tick();
}

void osal_task_create(const char *task_name, osal_task_function_t task_func, void *arg, uint8_t *stack_buffer, size_t stack_size, osal_task_priority_t priority)
{
    (void)task_name;
    (void)stack_buffer;
    (void)stack_size;
    (void)priority;

    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_active_workers++;
    }

    std::thread([task_func, arg]()
    {
        {
            std::lock_guard<std::mutex> lock(g_mutex);
            g_threads[std::this_thread::get_id()] = {g_current_tick.load(std::memory_order_relaxed), false};
        }
        
        task_func(arg);
    }).detach();
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
    return g_is_running.load();
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
    std::unique_lock<std::mutex> lock(g_mutex);

    auto tid = std::this_thread::get_id();
    uint32_t target_time = *last_wake_time + period_ms;

    g_threads[tid].target_wake_time = target_time;
    g_threads[tid].is_sleeping = true;

    g_active_workers--;

    if (g_active_workers == 0)
    {
        g_cv_god.notify_one();
    }

    g_cv_workers.wait(lock, [&]{ return g_current_tick >= g_threads[tid].target_wake_time || !osal_task_should_run(); });

    *last_wake_time += period_ms;

    return true;
}

void hal_time_init(void)
{
    (void)0;
}

uint32_t hal_time_get_ms_since_boot(void)
{
    return g_current_tick.load(std::memory_order_relaxed);
}

uint32_t hal_time_get_us_since_boot(void)
{
    return hal_time_get_ms_since_boot() * 1000;
}

void hal_time_sleep_ms(uint32_t ms)
{
    (void)ms;
}

void hal_time_sleep_us(uint32_t us)
{
    (void)us;
}