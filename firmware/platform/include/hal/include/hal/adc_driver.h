#ifndef _ADC_DRIVER_H
#define _ADC_DRIVER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef uint8_t hal_adc_channel_t; /** ADC channel definition */

/**
 * @brief Initialize ADC
 */
void hal_adc_init_all(void);

/**
 * @brief Initialize ADC for given input
 *
 * @param channel Channel to initialize
 */
void hal_adc_init_channel(hal_adc_channel_t channel);

/**
 * @brief Reads ADC value. To convert that value using proportion, use the following formula: (delta_value) / (delta_voltage) * (voltage - voltage_min) + value_min.
 *
 * @param channel Channel to read from
 * @return Voltage in volts. 0 if the channel is not initialized or invalid.
 */
float hal_adc_channel_read_voltage(hal_adc_channel_t channel);

#ifdef __cplusplus
}
#endif

#endif