#ifndef _STDIO_DRIVER_H
#define _STDIO_DRIVER_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the board
 */
void hal_stdio_init(void);

/**
 * @brief Check if USB is connected to the board
 *
 * @return True if USB is connected, false otherwise
 */
bool hal_stdio_is_usb_connected(void);

/**
 * @brief Prints message to stdio
 *
 * @param fmt Format
 * @param ... Params
 */
void hal_stdio_printf(const char *fmt, ...);

/**
 * @brief Sends byte buffer to stdio
 *
 * @param buffer Buffer of bytes to send
 * @param len Length of buffer
 */
void hal_stdio_send_buffer(const uint8_t *buffer, size_t len);

/**
 * @brief Read byte from stdio
 *
 * @param byte Byte that was read
 * @return True if byte was read, false if no byte was available
 */
bool hal_stdio_read_byte(uint8_t *byte);

#ifdef __cplusplus
}
#endif

#endif