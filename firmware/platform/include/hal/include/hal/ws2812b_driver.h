#ifndef _W2812B_DRIVER_H
#define _W2812B_DRIVER_H

/**
 * REF: https://www.youtube.com/watch?v=rHoFqKGOPRI
 */

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int hal_ws2812b_color_t; // WS2812 color represented as a 32-bit unsigned integer (0x00GGRRBB)

#define HAL_WS2812B_COLOR(r, g, b) (((unsigned int)(r) << 8) | ((unsigned int)(g) << 16) | (unsigned int)(b)) // Macro to create a color from RGB values

/**
 * @brief Initializes WS2812 LED strip
 *
 * @param pin Pin number
 * @param rgbw true if RGBW LEDs are used, false if RGB LEDs are used
 */
void hal_ws2812b_init(uint8_t pin, bool rgbw);

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