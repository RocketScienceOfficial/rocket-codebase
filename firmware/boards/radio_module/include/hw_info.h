#ifndef _HW_INFO_H
#define _HW_INFO_H

#include "../../shared/lora_config.h"
#include <hal/spi_driver.h>
#include <hal/uart_driver.h>

#ifdef __cplusplus
extern "C"
{
#endif

// --- BUSES ---
extern const hal_spi_bus_t g_cfg_spi;
extern const hal_uart_bus_t g_cfg_uart;

// --- LORA ---
extern const hal_gpio_pin_t g_cfg_lora_txen_pin;
extern const hal_gpio_pin_t g_cfg_lora_rxen_pin;
extern const hal_gpio_pin_t g_cfg_lora_cs_pin;
extern const hal_gpio_pin_t g_cfg_lora_dio1_pin;
extern const hal_gpio_pin_t g_cfg_lora_dio2_pin;
extern const hal_gpio_pin_t g_cfg_lora_reset_pin;
extern const hal_gpio_pin_t g_cfg_lora_busy_pin;

// --- INIT FUNCTION ---
void hw_init(void);

#ifdef __cplusplus
}
#endif

#endif