#ifndef _HW_INFO_H_
#define _HW_INFO_H_

#include "../../shared/lora_config.h"
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

// --- GPIO ---
extern const hal_gpio_pin_t g_cfg_button_pin;

// --- INIT FUNCTION ---
void hw_init(void);

#ifdef __cplusplus
}
#endif

#endif