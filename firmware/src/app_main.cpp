#include <hal/adc_driver.h>
#include <hal/flash_driver.h>
#include <hal/gpio_driver.h>
#include <hal/i2c_driver.h>
#include <hal/pwm_driver.h>
#include <hal/spi_driver.h>
#include <hal/stdio_driver.h>
#include <hal/time_driver.h>
#include <hal/uart_driver.h>
#include <osal/task.h>
#include <pubsub/MessageBus.h>
#include <modules/runner/ModulesRunner.h>
#include <board_config.h>

void hw_init()
{
    hal_stdio_init();
    hal_adc_init_all();
    hal_spi_init(OBC_SPI, OBC_SPI_MISO_PIN, OBC_SPI_MOSI_PIN, OBC_SPI_SCK_PIN, OBC_SPI_FREQ);
    hal_uart_init(OBC_UART, OBC_UART_RX, OBC_UART_TX, OBC_UART_FREQ);
    hal_flash_init();
    hal_time_init();
    
    osal_task_system_init();
}

void app_main()
{
    hw_init();

    PubSub::MessageBus::Init();
    ModulesRunner::startAllModules();
}