#include "hal/time_driver.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"

void hal_time_init(void)
{
}

uint32_t hal_time_get_ms_since_boot(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000ULL);
}

uint32_t hal_time_get_us_since_boot(void)
{
    return (uint32_t)esp_timer_get_time();
}

void hal_time_sleep_ms(uint32_t ms)
{
    esp_rom_delay_us(ms * 1000);
}

void hal_time_sleep_us(uint32_t us)
{
    esp_rom_delay_us(us);
}