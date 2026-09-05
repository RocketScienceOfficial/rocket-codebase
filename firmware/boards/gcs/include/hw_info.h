#ifndef _HW_INFO_H_
#define _HW_INFO_H_

#include <hal/spi_driver.h>
#include <hal/i2c_driver.h>
#include <hal/uart_driver.h>

#ifdef __cplusplus
extern "C"
{
#endif

// --- BUSES ---
extern const hal_spi_bus_t g_cfg_spi;
extern const hal_i2c_bus_t g_cfg_i2c;
extern const hal_uart_bus_t g_cfg_uart;

// --- LORA ---
extern const hal_gpio_pin_t g_cfg_lora_cs_pin;
extern const hal_gpio_pin_t g_cfg_lora_dio0_pin;
extern const hal_gpio_pin_t g_cfg_lora_reset_pin;

#define CFG_LORA_FREQ 433
#define CFG_LORA_BANDWIDTH 250
#define CFG_LORA_SF 7
#define CFG_LORA_TX_POWER 17

// --- GPIO ---
extern const hal_gpio_pin_t g_cfg_button_pin;

// --- DATALINK ---
#define CFG_LORA_SRC_ID 0xDF
#define CFG_LORA_DST_ID 0x11

// --- INIT FUNCTION ---
void hw_init(void);

#ifdef __cplusplus
}
#endif

#endif