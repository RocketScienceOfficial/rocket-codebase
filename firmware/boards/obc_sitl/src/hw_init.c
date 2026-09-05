#include <hal/stdio_driver.h>
#include <hal/adc_driver.h>
#include <hal/flash_driver.h>
#include <hal/time_driver.h>
#include <hw_info.h>

// -- INIT FUNCTION ---
void hw_init(void)
{
    hal_stdio_init();
    hal_adc_init_all();
    hal_flash_init();
    hal_time_init();
}