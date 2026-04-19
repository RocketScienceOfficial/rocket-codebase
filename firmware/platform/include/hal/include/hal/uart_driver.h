#ifndef _UART_DRIVER_H
#define _UART_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize UART
 *
 * @param bus UART bus
 * @param rx RX pin
 * @param tx TX pin
 * @param baudrate Baud rate
 */
void hal_uart_init(uint8_t bus, uint8_t rx, uint8_t tx, uint32_t baudrate);

/**
 * @brief Check if UART is ready to write
 * 
 * @param bus UART bus
 * @return true if UART is ready to write, false otherwise
 */
bool hal_uart_is_writable(uint8_t bus);

/**
 * @brief Write to UART with blocking
 *
 * @param bus UART bus
 * @param data Data pointer to write
 * @param size Size of data
 */
void hal_uart_write(uint8_t bus, const uint8_t *data, size_t size);

/**
 * @brief Check if UART data in FIFO is available
 *
 * @param bus UART bus
 * @return true if data is available, false otherwise
 */
bool hal_uart_fifo_available(uint8_t bus);

/**
 * @brief Read from UART with blocking
 *
 * @param bus UART bus
 * @param buffer Data buffer
 * @param bufSize Size of the buffer
 * @return Number of bytes read
 */
size_t hal_uart_read_fifo(uint8_t bus, uint8_t *buffer, size_t bufSize);

#ifdef __cplusplus
}
#endif

#endif