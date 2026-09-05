#ifndef _HW_INFO_H_
#define _HW_INFO_H_

#include <hal/waveform_driver.h>
#include <hal/spi_driver.h>
#include <hal/uart_driver.h>
#include <hal/adc_driver.h>
#include <hal/pwm_driver.h>

#ifdef __cplusplus
extern "C"
{
#endif

// --- BUSES ---
extern const hal_spi_bus_t g_cfg_spi;
extern const hal_uart_bus_t g_cfg_uart;

// --- PINS ---
extern const hal_waveform_channel_t g_cfg_led_channel;
extern const hal_gpio_pin_t g_cfg_led_pin;
extern const hal_pwm_timer_t g_cfg_buzzer_timer;
extern const hal_pwm_channel_t g_cfg_buzzer_channel;
extern const hal_gpio_pin_t g_cfg_buzzer_pin;
extern const hal_gpio_pin_t g_cfg_ign_en_1_pin;
extern const hal_gpio_pin_t g_cfg_ign_en_2_pin;
extern const hal_gpio_pin_t g_cfg_ign_en_3_pin;
extern const hal_gpio_pin_t g_cfg_ign_en_4_pin;
extern const hal_adc_channel_t g_cfg_ign_det_1_channel;
extern const hal_adc_channel_t g_cfg_ign_det_2_channel;
extern const hal_adc_channel_t g_cfg_ign_det_3_channel;
extern const hal_adc_channel_t g_cfg_ign_det_4_channel;
extern const hal_gpio_pin_t g_cfg_cs_h3lis_pin;
extern const hal_gpio_pin_t g_cfg_cs_lsm_pin;
extern const hal_gpio_pin_t g_cfg_cs_mmc_pin;
extern const hal_gpio_pin_t g_cfg_cs_bmi_acc_pin;
extern const hal_gpio_pin_t g_cfg_cs_bmi_gyro_pin;
extern const hal_gpio_pin_t g_cfg_cs_ms56_pin;
extern const hal_gpio_pin_t g_cfg_cs_neo_pin;
extern const hal_gpio_pin_t g_cfg_cs_ads_pin;
extern const hal_gpio_pin_t g_cfg_vbat_pin;
extern const hal_gpio_pin_t g_cfg_5v_pin;
extern const hal_gpio_pin_t g_cfg_3v3_pin;

// --- ADC / CALIB ---
#define CFG_EXTERNAL_ADC_VREF 2.5f
#define CFG_EXTERNAL_ADC_CALIB_SCALE 11.0f
#define CFG_EXTERNAL_ADC_CALIB_OFFSET 0.1f
#define CFG_INTERNAL_ADC_CALIB_SCALE 1.035f
#define CFG_INTERNAL_ADC_CALIB_OFFSET 0.036f

// --- INIT FUNCTION ---
void hw_init(void);

#ifdef __cplusplus
}
#endif

#endif