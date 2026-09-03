#include <hal/stdio_driver.h>
#include <hal/flash_driver.h>
#include <hal/time_driver.h>
#include <waveform_utils_pico.h>
#include <hw_info.h>

// --- DECLARATIONS ---
const hal_spi_bus_t g_cfg_spi = 0;
const hal_uart_bus_t g_cfg_uart = 0;

const hal_waveform_channel_t g_cfg_led_channel = HAL_WAVEFORM_PICO_CHANNEL_CREATE(0, 0);
const hal_gpio_pin_t g_cfg_led_pin = 15;
const hal_pwm_timer_t g_cfg_buzzer_timer = 3;
const hal_pwm_channel_t g_cfg_buzzer_channel = 0;
const hal_gpio_pin_t g_cfg_buzzer_pin = 22;
const hal_gpio_pin_t g_cfg_ign_en_1_pin = 21;
const hal_gpio_pin_t g_cfg_ign_en_2_pin = 18;
const hal_gpio_pin_t g_cfg_ign_en_3_pin = 20;
const hal_gpio_pin_t g_cfg_ign_en_4_pin = 19;
const hal_adc_channel_t g_cfg_ign_det_1_channel = 3;
const hal_adc_channel_t g_cfg_ign_det_2_channel = 2;
const hal_adc_channel_t g_cfg_ign_det_3_channel = 1;
const hal_adc_channel_t g_cfg_ign_det_4_channel = 0;
const hal_gpio_pin_t g_cfg_cs_h3lis_pin = 0;
const hal_gpio_pin_t g_cfg_cs_lsm_pin = 1;
const hal_gpio_pin_t g_cfg_cs_mmc_pin = 9;
const hal_gpio_pin_t g_cfg_cs_bmi_acc_pin = 6;
const hal_gpio_pin_t g_cfg_cs_bmi_gyro_pin = 5;
const hal_gpio_pin_t g_cfg_cs_ms56_pin = 7;
const hal_gpio_pin_t g_cfg_cs_neo_pin = 12;
const hal_gpio_pin_t g_cfg_cs_ads_pin = 13;
const hal_gpio_pin_t g_cfg_vbat_pin = 25;
const hal_gpio_pin_t g_cfg_5v_pin = 23;
const hal_gpio_pin_t g_cfg_3v3_pin = 24;

// --- INTERNAL CONFIG ---
#define SPI_FREQUENCY 5 * 1000 * 1000
#define SPI_SCK_PIN 2
#define SPI_MOSI_PIN 3
#define SPI_MISO_PIN 4

#define UART_FREQUENCY 1843200
#define UART_TX 16
#define UART_RX 17

// -- INIT FUNCTION ---
void hw_init(void)
{
    hal_stdio_init();
    hal_adc_init_all();
    hal_spi_init_bus(g_cfg_spi, SPI_MISO_PIN, SPI_MOSI_PIN, SPI_SCK_PIN, SPI_FREQUENCY);
    hal_uart_init_bus(g_cfg_uart, UART_RX, UART_TX, UART_FREQUENCY);
    hal_flash_init();
    hal_time_init();
}