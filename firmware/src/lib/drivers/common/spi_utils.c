#include "spi_utils.h"
#include <hal/gpio_driver.h>

void spi_utils_cs_init(uint8_t cs)
{
    hal_gpio_init_pin(cs, GPIO_OUTPUT);

    spi_utils_cs_deselect(cs);
}

void spi_utils_cs_select(uint8_t cs)
{
    hal_gpio_set_pin_state(cs, GPIO_LOW);
}

void spi_utils_cs_deselect(uint8_t cs)
{
    hal_gpio_set_pin_state(cs, GPIO_HIGH);
}