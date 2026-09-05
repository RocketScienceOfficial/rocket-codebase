#include <hal/stdio_driver.h>
#include <hal/time_driver.h>
#include <hw_info.h>

void hw_init(void)
{
    hal_stdio_init();
    hal_time_init();
}