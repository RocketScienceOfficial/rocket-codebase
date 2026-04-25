#include <hal/stdio_driver.h>
#include <hal/i2c_driver.h>
#include <hal/uart_driver.h>
#include <hal/time_driver.h>
#include <board_config.h>

void hw_init(void)
{
    hal_stdio_init();
    hal_uart_init(CFG_UART, CFG_UART_TX, CFG_UART_RX, CFG_UART_FREQ);
    hal_i2c_init(CFG_I2C, CFG_I2C_SDA_PIN, CFG_I2C_SCL_PIN, CFG_I2C_FREQUENCY);
    hal_time_init();
}