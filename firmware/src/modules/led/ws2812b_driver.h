#ifndef _W2812B_DRIVER_H
#define _W2812B_DRIVER_H

/**
 * REF: https://www.youtube.com/watch?v=rHoFqKGOPRI
 */

#include <hal/gpio_driver.h>
#include <hal/waveform_driver.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t ws2812b_color_t; /** WS2812 color represented as a 32-bit unsigned integer (0x00GGRRBB) */

#define WS2812B_COLOR_RGB(r, g, b) (((uint32_t)(g) << 24) | ((uint32_t)(r) << 16) | ((uint32_t)(b) << 8)) /** Macro to create a color from RGB values (GRB order) */

/**
 * @brief Initializes WS2812 LED strip
 *
 * @param channel Waveform channel to use
 * @param pin Pin number
 * @param rgbw true if RGBW LEDs are used, false if RGB LEDs are used
 */
void ws2812b_init(hal_waveform_channel_t channel, hal_gpio_pin_t pin, bool rgbw);

/**
 * @brief Sets colors of all LEDs
 *
 * @param channel Waveform channel to use
 * @param colors Array of colors
 * @param count Number of colors
 */
void ws2812b_set_colors(hal_waveform_channel_t channel, const ws2812b_color_t *colors, size_t count);

#ifdef __cplusplus
}
#endif

#endif