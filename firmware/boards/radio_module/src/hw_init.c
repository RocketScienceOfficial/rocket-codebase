#include <hal/stdio_driver.h>
#include <hal/time_driver.h>
#include <hw_info.h>

// --- DECLARATIONS ---
const hal_spi_bus_t g_cfg_spi = 0;
const hal_uart_bus_t g_cfg_uart = 0;

const hal_gpio_pin_t g_cfg_lora_txen_pin = 28;
const hal_gpio_pin_t g_cfg_lora_rxen_pin = 27;
const hal_gpio_pin_t g_cfg_lora_cs_pin = 5;
const hal_gpio_pin_t g_cfg_lora_dio1_pin = 6;
const hal_gpio_pin_t g_cfg_lora_dio2_pin = 29;
const hal_gpio_pin_t g_cfg_lora_reset_pin = 1;
const hal_gpio_pin_t g_cfg_lora_busy_pin = 0;

// --- INTERNAL CONFIG ---
#define SPI_FREQUENCY 500 * 1000
#define SPI_SCK_PIN 2
#define SPI_MOSI_PIN 3
#define SPI_MISO_PIN 4

#define UART_FREQUENCY 1843200
#define UART_TX 12
#define UART_RX 13

void hw_init(void)
{
    hal_stdio_init();
    hal_spi_init_bus(g_cfg_spi, SPI_SCK_PIN, SPI_MOSI_PIN, SPI_MISO_PIN, SPI_FREQUENCY);
    hal_uart_init_bus(g_cfg_uart, UART_RX, UART_TX, UART_FREQUENCY);
    hal_time_init();
}