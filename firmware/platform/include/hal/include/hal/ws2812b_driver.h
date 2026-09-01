#ifndef _W2812B_DRIVER_H
#define _W2812B_DRIVER_H

/**
 * REF: https://www.youtube.com/watch?v=rHoFqKGOPRI
 */

#include "gpio_driver.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t hal_ws2812b_color_t; // WS2812 color represented as a 32-bit unsigned integer (0x00GGRRBB)

#define HAL_WS2812B_COLOR(r, g, b) (((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b)) // Macro to create a color from RGB values

/**
 * @brief Initializes WS2812 LED strip
 *
 * @param pin Pin number
 * @param rgbw true if RGBW LEDs are used, false if RGB LEDs are used
 */
void hal_ws2812b_init(hal_gpio_pin_t pin, bool rgbw);

/**
 * @brief Sets colors of all LEDs
 *
 * @param colors Array of colors
 * @param count Number of colors
 */
void hal_ws2812b_set_colors(const hal_ws2812b_color_t *colors, size_t count);

#ifdef __cplusplus
}
#endif

#endif