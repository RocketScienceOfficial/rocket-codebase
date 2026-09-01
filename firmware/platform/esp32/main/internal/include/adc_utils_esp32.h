#ifndef _ADC_UTILS_ESP32_H
#define _ADC_UTILS_ESP32_H

#include "hal/adc_driver.h"

/** @brief Packs an ESP32 ADC unit (1 or 2) and its channel index into a single hal_adc_channel_t. */
#define HAL_ADC_ESP32_ENCODE_CHANNEL(unit, channel) ((hal_adc_channel_t)(((channel) << 1) | (((unit) - 1) & 0x1)))

/** @brief Decodes the ADC unit (1 or 2) from a HAL_ADC_ESP32_ENCODE_CHANNEL value. */
#define HAL_ADC_ESP32_DECODE_UNIT(encoded) (((encoded) & 0x1) + 1)

/** @brief Decodes the channel index from a HAL_ADC_ESP32_ENCODE_CHANNEL value. */
#define HAL_ADC_ESP32_DECODE_CHANNEL(encoded) ((encoded) >> 1)

#endif