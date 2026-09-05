#ifndef _HW_INFO_H
#define _HW_INFO_H

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

#define CFG_LORA_FREQ 433
#define CFG_LORA_BANDWIDTH 250
#define CFG_LORA_SF 7
#define CFG_LORA_TX_POWER 17

// --- DATALINK ---
#define CFG_LORA_SRC_ID 0x11
#define CFG_LORA_DST_ID 0xDF

// --- INIT FUNCTION ---
void hw_init(void);

#ifdef __cplusplus
}
#endif

#endif