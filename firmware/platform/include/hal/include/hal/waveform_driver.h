#ifndef _WAVEFORM_DRIVER_H
#define _WAVEFORM_DRIVER_H

#include "gpio_driver.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t hal_waveform_channel_t; /** Waveform channel (platform specific) */

/**
 * @brief Initializes a waveform channel (in timing ratio 2:5:3)
 * 
 * @param channel Waveform channel
 * @param pin GPIO pin to output the waveform
 * @param symbol_rate_hz Number of symbols transmitted per second
 * @param symbol_bits Number of valid bits in each symbol (1-32)
 */
void hal_waveform_253_init(hal_waveform_channel_t channel, hal_gpio_pin_t pin, uint32_t symbol_rate_hz, uint8_t symbol_bits);

/**
 * @brief Sends a waveform on a channel, blocking (in timing ratio 2:5:3)
 * 
 * @param channel Waveform channel
 * @param symbols Pointer to waveform symbols (MSB first, 1-32 valid bits per symbol)
 * @param count Number of symbols
 */
void hal_waveform_253_send_blocking(hal_waveform_channel_t channel, const uint32_t *symbols, size_t count);

#ifdef __cplusplus
}
#endif

#endif