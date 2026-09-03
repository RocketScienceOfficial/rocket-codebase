#include <hal/stdio_driver.h>
#include <hal/time_driver.h>
#include <hw_info.h>

// --- DECLARATIONS ---
const hal_spi_bus_t g_cfg_spi = 2;
const hal_i2c_bus_t g_cfg_i2c = 0;
const hal_uart_bus_t g_cfg_uart = 1;
const hal_gpio_pin_t g_cfg_button_pin = 38;
const hal_gpio_pin_t g_cfg_lora_cs_pin = 18;
const hal_gpio_pin_t g_cfg_lora_dio0_pin = 26;
const hal_gpio_pin_t g_cfg_lora_reset_pin = 23;

// --- INTERNAL CONFIG ---
#define SPI_FREQUENCY 500 * 1000
#define SPI_SCK_PIN 5
#define SPI_MOSI_PIN 27
#define SPI_MISO_PIN 19

#define UART_FREQUENCY 9600
#define UART_TX 34
#define UART_RX 12

#define I2C_FREQUENCY 400 * 1000
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22

// --- INIT FUNCTION ---
void hw_init(void)
{
    hal_stdio_init();
    hal_spi_init_bus(g_cfg_spi, SPI_SCK_PIN, SPI_MOSI_PIN, SPI_MISO_PIN, SPI_FREQUENCY);
    hal_uart_init_bus(g_cfg_uart, UART_TX, UART_RX, UART_FREQUENCY);
    hal_i2c_init_bus(g_cfg_i2c, I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQUENCY);
    hal_time_init();
}