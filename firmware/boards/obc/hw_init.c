#include <hal/stdio_driver.h>
#include <hal/adc_driver.h>
#include <hal/spi_driver.h>
#include <hal/uart_driver.h>
#include <hal/flash_driver.h>
#include <hal/time_driver.h>
#include <board_config.h>

void hw_init(void)
{
    hal_stdio_init();
    hal_adc_init_all();
    hal_spi_init(OBC_SPI, OBC_SPI_MISO_PIN, OBC_SPI_MOSI_PIN, OBC_SPI_SCK_PIN, OBC_SPI_FREQ);
    hal_uart_init(OBC_UART, OBC_UART_RX, OBC_UART_TX, OBC_UART_FREQ);
    hal_flash_init();
    hal_time_init();
}