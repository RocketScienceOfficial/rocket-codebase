#include "OLEDModule.h"
#include <board_config.h>
#include <hal/i2c_driver.h>
#include <hal/gpio_driver.h>
#include <hal/time_driver.h>
#include <string.h>

static uint8_t u8x8_gpio_and_delay_hal(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    (void)u8x8;
    (void)arg_ptr;

    switch (msg)
    {
    case U8X8_MSG_DELAY_MILLI:
        hal_time_sleep_ms(arg_int);
        break;
    case U8X8_MSG_DELAY_10MICRO:
        hal_time_sleep_us(arg_int * 10);
        break;
    case U8X8_MSG_DELAY_100NANO:
        hal_time_sleep_us(1);
        break;
    }

    return 1;
}

static uint8_t u8x8_byte_hal_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    static uint8_t buffer[128];
    static uint8_t buf_idx = 0;

    switch (msg)
    {
    case U8X8_MSG_BYTE_SEND:
        memcpy(&buffer[buf_idx], arg_ptr, arg_int);
        buf_idx += arg_int;
        break;

    case U8X8_MSG_BYTE_INIT:
        break;

    case U8X8_MSG_BYTE_START_TRANSFER:
        buf_idx = 0;
        break;

    case U8X8_MSG_BYTE_END_TRANSFER:
        hal_i2c_transfer(CFG_I2C, u8x8_GetI2CAddress(u8x8) >> 1, buffer, buf_idx, NULL, 0);
        break;
    }

    return 1;
}

void OLEDModule::initDisplay()
{
    u8g2_Setup_ssd1306_i2c_128x64_noname_f(&m_Display, U8G2_R0, u8x8_byte_hal_i2c, u8x8_gpio_and_delay_hal);

    hal_gpio_init_pin(CFG_BUTTON_PIN, GPIO_INPUT);
}

bool OLEDModule::shouldChangeState()
{
    return hal_gpio_get_pin_state(CFG_BUTTON_PIN) == GPIO_LOW;
}