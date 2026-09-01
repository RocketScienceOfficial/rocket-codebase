#ifndef _UART_DRIVER_H
#define _UART_DRIVER_H

#include "gpio_driver.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef uint8_t hal_uart_bus_t; /** UART bus definition */

/**
 * @brief Initialize UART
 *
 * @param bus UART bus
 * @param rx RX pin
 * @param tx TX pin
 * @param baudrate Baud rate
 */
void hal_uart_init_bus(hal_uart_bus_t bus, hal_gpio_pin_t rx, hal_gpio_pin_t tx, uint32_t baudrate);

/**
 * @brief Check if UART is ready to write
 *
 * @param bus UART bus
 * @return true if UART is ready to write, false otherwise
 */
bool hal_uart_is_writable(hal_uart_bus_t bus);

/**
 * @brief Write to UART with blocking
 *
 * @param bus UART bus
 * @param data Data pointer to write
 * @param size Size of data
 */
void hal_uart_write(hal_uart_bus_t bus, const uint8_t *data, size_t size);

/**
 * @brief Check if UART data in FIFO is available
 *
 * @param bus UART bus
 * @return true if data is available, false otherwise
 */
bool hal_uart_fifo_available(hal_uart_bus_t bus);

/**
 * @brief Read from UART with blocking
 *
 * @param bus UART bus
 * @param buffer Data buffer
 * @param bufSize Size of the buffer
 * @return Number of bytes read
 */
size_t hal_uart_read_fifo(hal_uart_bus_t bus, uint8_t *buffer, size_t bufSize);

#ifdef __cplusplus
}
#endif

#endif