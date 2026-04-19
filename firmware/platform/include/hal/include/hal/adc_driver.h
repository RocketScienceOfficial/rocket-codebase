#ifndef _ADC_DRIVER_H
#define _ADC_DRIVER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize ADC
 */
void hal_adc_init_all(void);

/**
 * @brief Initialize ADC for given input
 *
 * @param pin Pin to initialize
 */
void hal_adc_init_pin(uint8_t pin);

/**
 * @brief Reads ADC value. To convert that value using proportion, use the following formula: (delta_value) / (delta_voltage) * (voltage - voltage_min) + value_min.
 *
 * @param pin Pin to read from
 * @return Voltage in volts. 0 if the pin is not initialized or invalid.
 */
float hal_adc_read_voltage(uint8_t pin);

#ifdef __cplusplus
}
#endif

#endif