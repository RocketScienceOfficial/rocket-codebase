#include "hal/time_driver.h"
#include "pico/time.h"

void hal_time_init(void)
{
}

uint32_t hal_time_get_ms_since_boot(void)
{
	return to_ms_since_boot(get_absolute_time());
}

uint32_t hal_time_get_us_since_boot(void)
{
	return to_us_since_boot(get_absolute_time());
}

void hal_time_sleep_ms(uint32_t ms)
{
	sleep_ms(ms);
}

void hal_time_sleep_us(uint32_t us)
{
	sleep_us(us);
}