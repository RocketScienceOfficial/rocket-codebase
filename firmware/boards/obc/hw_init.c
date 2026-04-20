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
    hal_spi_init(CFG_SPI, CFG_SPI_MISO_PIN, CFG_SPI_MOSI_PIN, CFG_SPI_SCK_PIN, CFG_SPI_FREQ);
    hal_uart_init(CFG_UART, CFG_UART_RX, CFG_UART_TX, CFG_UART_FREQ);
    hal_flash_init();
    hal_time_init();
}